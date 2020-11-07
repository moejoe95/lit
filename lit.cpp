#include <filesystem>
#include <fstream>
#include <iostream>
#include <regex>
#include <string>
#include <vector>

#include "branch.hpp"
#include "commit.hpp"
#include "constants.hpp"
#include "exec.hpp"
#include "file_utils.hpp"

namespace fs = std::filesystem;
using std::cout;
using std::endl;
using std::string;

void print_help_message() {
  cout << "usage: lit <command> [<commit_id>] \n\n"
          "command is one of: \n"
          "  help \n"
          "    Display this message.\n"
          "  status \n"
          "    Display current status of the lit repository.\n"
          "  init \n"
          "    Initialize a lit repository.\n"
          "  commit <id>\n"
          "    Creates a new commit containing all changes.\n"
          "  show <id>\n"
          "    Lets you inspect a commit.\n"
          "  checkout <id>\n"
          "    Resets the state of all files to a given commit' state.\n"
          "  merge <id>\n"
          "    Starts merging process with the currently checked out "
          "commit and a specified commit.\n"
          "  log \n"
          "    Displays a commit graph.\n"
          "  branches \n"
          "    List active branches."
       << endl;
}

int print_error(const string &message) {
  cout << message << endl << endl;
  print_help_message();
  return -1;
}

void lit_init(fs::path cwd) {
  string lit_path = cwd / LIT_DIR;
  if (fs::exists(lit_path)) {
    cout << "directory already initialized as a lit repository." << endl;
  } else {
    commit init_commit{cwd, "initialize lit repository"};
  }
}

std::string lit_checkout(fs::path cwd, const std::string &rev) {
  fs::path lit_dir = cwd / LIT_DIR;
  int parent_id = read_int_from_file(lit_dir / rev / COMMIT_PARENT_ID_FILE);

  fs::path patch_file = lit_dir / rev / COMMIT_PATCH;

  std::vector<string> args{"-ruN", "-d", cwd, "<", patch_file};
  exec cmd("patch", args);
  cmd.run();

  // reset head
  write_to_file(lit_dir / COMMIT_HEAD, parent_id);

  std::string parent_rev_dir = REVISION_PX + std::to_string(parent_id);
  return parent_rev_dir;
}

void create_commit(fs::path cwd, std::string message) {
  commit commit{cwd, message};
  commit.create_commit();
}

int main(int argc, char **argv) {

  if (argc < 2) {
    return print_error("too few arguments given.");
  }

  string command = argv[1];
  fs::path cwd = fs::current_path();
  fs::path lit_dir = cwd / LIT_DIR;

  if (!fs::exists(lit_dir)) {
    if (command != CMD_HELP && command != CMD_INIT) {
      cout << "this directory is not initialized." << endl;
      return -1;
    }
  }

  int latest_id = read_int_from_file(lit_dir / COMMIT_ID_FILE);
  string rev_dir = REVISION_PX + std::to_string(latest_id);

  if (command.compare(CMD_HELP) == 0) {
    print_help_message();
  } else if (command.compare(CMD_STATUS) == 0) {

    int head_id = read_int_from_file(lit_dir / COMMIT_HEAD);
    cout << "on commit: r" << head_id << endl;

    // TODO compare with HEAD, not with latest

    // list new files
    std::regex exclude("^.lit*");
    compare_directories(cwd, lit_dir / rev_dir, exclude, "NEW");

    for (const auto &entry : fs::directory_iterator(cwd)) {
      string filename = entry.path().filename().u8string();
      if (!regex_match(filename, exclude)) {
        if (fs::exists(lit_dir / rev_dir / filename)) {

          std::vector<string> args{"-q", cwd / filename,
                                   lit_dir / rev_dir / filename};
          exec cmd("diff", args);

          string response = cmd.run();
          if (!response.empty()) {
            cout << "MOD: " << filename << endl;
          }
        }
      }
    }

    // list deleted files
    exclude = std::regex("(^.lit*|^commit.*)");
    compare_directories(lit_dir / rev_dir, cwd, exclude, "DEL");

    // TODO check for modified files

  } else if (command.compare(CMD_INIT) == 0) {

    lit_init(cwd);

  } else if (command.compare(CMD_COMMIT) == 0) {

    if (argc < 3) {
      cout << "no commit message given." << endl;
    } else {
      create_commit(cwd, argv[2]);
    }
  } else if (command.compare(CMD_SHOW) == 0) {

    string revision;
    if (argc < 3) {
      int head_id = read_int_from_file(lit_dir / COMMIT_HEAD);
      revision = REVISION_PX + std::to_string(head_id);
    } else {
      revision = argv[2];
    }

  } else if (command.compare(CMD_CHECKOUT) == 0) {

    string rev_to_check;
    int head_id = read_int_from_file(lit_dir / COMMIT_HEAD);
    rev_to_check = REVISION_PX + std::to_string(head_id);

    if (head_id != latest_id) {
      std::regex exclude_commit("(^commit.*)");
      delete_files(lit_dir / rev_to_check, exclude_commit);
    }

    if (argc >= 3) {
      rev_to_check = argv[2];
    }

    // delete files because no new branch was created
    // TODO check if rev_to_check is head of branch, not latest commit
    if (rev_to_check == rev_dir) {
      write_to_file(lit_dir / COMMIT_HEAD, latest_id);
      std::regex exclude("(^.lit*|^commit.*)");
      copy_files_exclude(lit_dir / rev_dir, cwd, exclude, true);
      return 0;
    }

    while (rev_to_check != rev_dir) {
      if (rev_dir == "r0") {
        cout << "revision: " << rev_to_check << " not found" << endl;
        write_to_file(lit_dir / COMMIT_HEAD, head_id);
        break;
      }
      rev_dir = lit_checkout(cwd, rev_dir);
    }

    // copy files needed if new branch created in next step
    std::regex exclude_lit("(^.lit*)");
    copy_files_exclude(cwd, lit_dir / rev_to_check, exclude_lit, false);

  } else if (command.compare(CMD_MERGE) == 0) {
    // TODO
  } else if (command.compare(CMD_LOG) == 0) {

    while (latest_id >= 0) {
      rev_dir = REVISION_PX + std::to_string(latest_id);
      string message =
          read_string_from_file(lit_dir / rev_dir / COMMIT_MESSAGE_FILE);
      cout << "o " << rev_dir << " " << message << endl;
      latest_id--;
    }

  } else if (command.compare(CMD_BRANCHES) == 0) {
    branch branch{cwd};
    std::set<string> branches = branch.get_active_branches();
    for (auto b : branches) {
      cout << b << std::endl;
    }
  } else {
    return print_error("command not known.");
  }

  return 0;
}
