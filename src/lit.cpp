#include <filesystem>
#include <fstream>
#include <iostream>
#include <regex>
#include <string>
#include <vector>

#include "headers/branch.hpp"
#include "headers/commit.hpp"
#include "headers/commit_graph.hpp"
#include "headers/constants.hpp"
#include "headers/exec.hpp"
#include "headers/file_utils.hpp"

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
    commit init_commit{cwd};
  }
}

std::string lit_checkout_next(fs::path cwd, const string &rev, int parent_id) {
  fs::path lit_dir = cwd / LIT_DIR;
  fs::path patch_file = lit_dir / rev / COMMIT_PATCH;

  std::vector<string> args{"-ruN", "-d", cwd, "<", patch_file};
  exec cmd("patch", args);
  cmd.run();

  // reset head
  write_to_file(lit_dir / COMMIT_HEAD, parent_id);

  std::string parent_rev_dir = REVISION_PX + std::to_string(parent_id);
  return parent_rev_dir;
}

std::string lit_checkout(fs::path cwd, const std::string &rev_to_check,
                         std::string rev_dir) {

  fs::path lit_dir = cwd / LIT_DIR;
  std::vector<int> parent_ids =
      read_vec_from_file(lit_dir / rev_dir / COMMIT_PARENT_ID_FILE);

  for (int p : parent_ids) {
    rev_dir = lit_checkout_next(cwd, rev_dir, p);
    if (rev_dir == "r0" || rev_to_check == rev_dir) {
      return rev_dir;
    } else {
      rev_dir = lit_checkout(cwd, rev_to_check, rev_dir);
    }
  }

  return rev_dir;
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
      return print_error("this directory is not initialized.");
    }
  }

  int latest_id = read_int_from_file(lit_dir / COMMIT_ID_FILE);
  string rev_dir = REVISION_PX + std::to_string(latest_id);
  int head_id = read_int_from_file(lit_dir / COMMIT_HEAD);
  string head_dir = REVISION_PX + std::to_string(head_id);

  if (command.compare(CMD_HELP) == 0) {

    print_help_message();

  } else if (command.compare(CMD_STATUS) == 0) {

    if (head_id >= 0)
      cout << "on commit: r" << head_id << endl;

    // list new files
    std::regex exclude("^.lit*");
    compare_directories(cwd, lit_dir / head_dir, exclude, "NEW");

    if (head_id < 0) {
      return 0;
    }

    for (const auto &entry : fs::directory_iterator(cwd)) {
      string filename = entry.path().filename().u8string();
      if (!regex_match(filename, exclude)) {
        if (fs::exists(lit_dir / head_dir / filename)) {

          std::vector<string> args{"-q", cwd / filename,
                                   lit_dir / head_dir / filename};
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
    compare_directories(lit_dir / head_dir, cwd, exclude, "DEL");

    // TODO check for modified files

  } else if (command.compare(CMD_INIT) == 0) {

    lit_init(cwd);

  } else if (command.compare(CMD_COMMIT) == 0) {

    create_commit(cwd, argv[2]);

  } else if (command.compare(CMD_SHOW) == 0) {

    if (argc >= 3) {
      head_dir = argv[2];
    }

    if (!fs::exists(lit_dir / head_dir)) {
      return print_error("Commit: " + head_dir + " does not exist.");
    }

    cout << read_string_from_file(lit_dir / head_dir / COMMIT_INFO_FILE);

    if (fs::exists(lit_dir / head_dir / COMMIT_PATCH)) {
      string patch = read_string_from_file(lit_dir / head_dir / COMMIT_PATCH);
      cout << patch.substr(patch.find_first_of('\n'), patch.size());
    }

  } else if (command.compare(CMD_CHECKOUT) == 0) {

    // get commit to checkout
    string rev_to_check;
    rev_to_check = REVISION_PX + std::to_string(head_id);
    if (argc >= 3) {
      rev_to_check = argv[2];
    }

    // check if revision exists
    if (!fs::exists(lit_dir / rev_to_check)) {
      return print_error("Commit: " + rev_to_check + " does not exist.");
    }

    branch branch{cwd};
    std::set<string> branches = branch.get_active_branches();

    if (branches.count(rev_to_check) < 1) {
      std::regex exclude_commit("(^commit.*)");
      delete_files(lit_dir / rev_to_check, exclude_commit);
    }

    bool rev_found = false;
    for (auto b : branches) {
      std::regex exclude("(^.lit*|^commit.*)");
      copy_files_exclude(lit_dir / b, cwd, exclude, true);
      if (rev_to_check == b) {
        head_id = std::stoi(b.substr(1, b.size()));
        write_to_file(lit_dir / COMMIT_HEAD, head_id);
        break;
      }
      rev_dir = lit_checkout(cwd, rev_to_check, b);
      if (rev_dir == rev_to_check) {
        rev_found = true;
        break;
      }
    }

    if (rev_dir == "r-1") {
      cout << "revision: " << rev_to_check << " not found" << endl;
      write_to_file(lit_dir / COMMIT_HEAD, head_id);
    }

    // copy files needed if new branch created in next step
    if (rev_found) {
      std::regex exclude_lit("(^.lit*)");
      copy_files_exclude(cwd, lit_dir / rev_to_check, exclude_lit, false);
    }

  } else if (command.compare(CMD_MERGE) == 0) {

    string rev_to_merge{argv[2]};
    std::vector<string> conflict_files;

    for (const auto &entry : fs::directory_iterator(lit_dir / rev_to_merge)) {
      string filename = entry.path().filename().u8string();
      string base_file = lit_dir / head_dir / filename;
      string to_merge_file = lit_dir / rev_to_merge / filename;

      std::regex exclude_commit_files(COMMIT_FILE_PATTERN);
      if (regex_match(filename, exclude_commit_files)) {
        continue;
      }

      if (!fs::exists(base_file)) {
        // file was created newly in branch
        fs::copy(to_merge_file, cwd);
      } else {
        std::vector<string> args{base_file, to_merge_file};
        exec cmd("diff", args);
        string ret = cmd.run();
        if (ret.empty()) {
          // files are both untouched, do nothing
        } else {
          // we have a conflict
          // TODO
        }
      }
    }

    if (conflict_files.empty()) {
      branch branch{cwd};
      branch.merge_branch(rev_to_merge);

      string message = "merge " + rev_to_merge + " into " + head_dir;
      commit merge_commit{cwd, message};
      merge_commit.create_merge_commit(rev_to_merge);
    } else {
      cout << "there are merge conflicts in following files:" << endl;
      for (auto f : conflict_files) {
        cout << "\t" << f << endl;
      }
    }
  } else if (command.compare(CMD_LOG) == 0) {

    commit_graph graph{cwd};
    graph.print_graph();

  } else if (command.compare(CMD_BRANCHES) == 0) {

    branch branch{cwd};
    branch.print_branches();

  } else {
    return print_error("command not known.");
  }

  return 0;
}
