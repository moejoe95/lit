#include <filesystem>
#include <fstream>
#include <iostream>
#include <regex>
#include <string>
#include <vector>

#include "headers/branch.hpp"
#include "headers/commit.hpp"
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
    commit init_commit{cwd, "initialize lit repository"};
  }
}

std::string lit_checkout_next(fs::path cwd, const std::string &rev) {
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

std::string lit_checkout(fs::path cwd, const std::string &rev_to_check,
                         std::string rev_dir) {
  while (rev_to_check != rev_dir) {
    if (rev_dir != "r0") {
      rev_dir = lit_checkout_next(cwd, rev_dir);
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
      cout << "this directory is not initialized." << endl;
      return -1;
    }
  }

  int latest_id = read_int_from_file(lit_dir / COMMIT_ID_FILE);
  string rev_dir = REVISION_PX + std::to_string(latest_id);
  int head_id = read_int_from_file(lit_dir / COMMIT_HEAD);
  string head_dir = REVISION_PX + std::to_string(head_id);

  if (command.compare(CMD_HELP) == 0) {
    print_help_message();
  } else if (command.compare(CMD_STATUS) == 0) {

    cout << "on commit: r" << head_id << endl;

    // list new files
    std::regex exclude("^.lit*");
    compare_directories(cwd, lit_dir / head_dir, exclude, "NEW");

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

    if (argc < 3) {
      cout << "no commit message given." << endl;
    } else {
      create_commit(cwd, argv[2]);
    }
  } else if (command.compare(CMD_SHOW) == 0) {

    if (argc >= 3) {
      head_dir = argv[2];
    }

    if (!fs::exists(lit_dir / head_dir)) {
      cout << "Commit: " << head_dir << " does not exist." << endl;
      return 0;
    }

    cout << read_string_from_file(lit_dir / head_dir / COMMIT_INFO_FILE);

    string patch = read_string_from_file(lit_dir / head_dir / COMMIT_PATCH);
    cout << patch.substr(patch.find_first_of('\n'), patch.size());

  } else if (command.compare(CMD_CHECKOUT) == 0) {

    string rev_to_check;
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

    branch branch{cwd};
    std::set<string> branches = branch.get_active_branches();
    bool rev_found = false;
    for (auto b : branches) {
      std::regex exclude("(^.lit*|^commit.*)");
      copy_files_exclude(lit_dir / b, cwd, exclude, true);
      rev_dir = lit_checkout(cwd, rev_to_check, b);
      if (rev_dir != "r0") {
        rev_found = true;
        break;
      }
    }

    if (rev_dir == "r0") {
      cout << "revision: " << rev_to_check << " not found" << endl;
      write_to_file(lit_dir / COMMIT_HEAD, head_id);
    }

    // copy files needed if new branch created in next step
    if (rev_found) {
      std::regex exclude_lit("(^.lit*)");
      copy_files_exclude(cwd, lit_dir / rev_to_check, exclude_lit, false);
    }

  } else if (command.compare(CMD_MERGE) == 0) {

    if (argc < 3) {
      cout << "no commit to merge given." << endl;
      return 0;
    }

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
        fs::copy(to_merge_file, lit_dir / head_dir);
      } else {
        std::vector<string> args{base_file, to_merge_file};
        exec cmd("diff", args);
        string ret = cmd.run();
        if (ret.empty()) {
          // files are both untouched
        } else {
          // we have a conflict
          conflict_files.push_back(to_merge_file);
        }
      }
    }

    if (conflict_files.empty()) {
      cout << "fast forward merge ..." << endl;
      branch branch{cwd};
      branch.merge_branch(rev_to_merge);
    } else {
      cout << "there are merge conflicts in following files:" << endl;
      for (auto f : conflict_files) {
        cout << "\t" << f << endl;
      }
    }
  } else if (command.compare(CMD_LOG) == 0) {

    branch branch{cwd};
    std::set<string> branches = branch.get_active_branches();

    std::vector<std::vector<int>> history;
    for (auto b : branches) {
      int id = read_int_from_file(lit_dir / b / COMMIT_PARENT_ID_FILE);
      std::vector<int> branch_hist;
      b = b.substr(1, b.size());
      branch_hist.push_back(std::stoi(b));
      while (id > 0) {
        string rev = REVISION_PX + std::to_string(id);
        branch_hist.push_back(id);
        int old_id = id;
        id = read_int_from_file(lit_dir / rev / COMMIT_PARENT_ID_FILE);
      }
      history.push_back(branch_hist);
    }

    while (latest_id > 0) {
      string rev = REVISION_PX + std::to_string(latest_id);
      string msg = read_string_from_file(lit_dir / rev / COMMIT_MESSAGE_FILE);

      int i = 0;
      bool is_brached = false;
      for (auto branch_hist : history) {
        for (int j = 0; j < i; j++)
          cout << " ";

        if (std::count(branch_hist.begin(), branch_hist.end(), latest_id)) {
          if (is_brached)
            cout << "_|";
          else
            cout << " o";
          is_brached = true;
        } else {
          if (latest_id == 1)
            cout << " o";
          else
            cout << " |";
        }
        i++;
      }

      cout << "\t\t" << msg << endl;
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
