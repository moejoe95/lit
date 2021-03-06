#include <filesystem>
#include <fstream>
#include <iostream>
#include <regex>
#include <string>
#include <vector>

#include "headers/branch.hpp"
#include "headers/checkout.hpp"
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
  cout << "usage: lit <command> \n\n"
          "command is one of: \n"
          "  help \n"
          "    Display this text.\n"
          "  status \n"
          "    Display current status of the lit repository.\n"
          "  init \n"
          "    Initialize a lit repository.\n"
          "  commit <message>\n"
          "    Creates a new commit with commit-message.\n"
          "  show <id>\n"
          "    Lets you inspect a commit.\n"
          "  checkout <id>\n"
          "    Resets the state of all files to a given commit state.\n"
          "  merge <id>\n"
          "    Starts merging process with the currently checked out "
          "commit and a specified commit.\n"
          "  log \n"
          "    Displays a commit graph."
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
    if (command != "help" && command != "init") {
      return print_error("this directory is not initialized.");
    }
  }

  int latest_id = read_int_from_file(lit_dir / COMMIT_ID_FILE);
  string rev_dir = REVISION_PX + std::to_string(latest_id);
  int head_id = read_int_from_file(lit_dir / COMMIT_HEAD);
  string head_dir = REVISION_PX + std::to_string(head_id);

  if (command == "help") {

    print_help_message();

  } else if (command == "status") {

    if (head_id >= 0)
      cout << "on commit: r" << head_id << endl;

    compare_directories(cwd, lit_dir / head_dir, RX_LIT_FILES, "NEW");

    if (head_id < 0) {
      return 0;
    }

    for (const auto &entry : fs::directory_iterator(cwd)) {
      string filename = entry.path().filename().u8string();
      if (!regex_match(filename, RX_LIT_FILES)) {
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
    compare_directories(lit_dir / head_dir, cwd, RX_LIT_COMMIT_FILES, "DEL");

    // TODO check for modified files

  } else if (command == "init") {

    lit_init(cwd);

  } else if (command == "commit") {

    create_commit(cwd, argv[2]);

  } else if (command == "show") {

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

  } else if (command == "checkout") {

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

    checkout checkout{cwd, head_dir, rev_to_check};
    checkout.create(rev_dir, head_id);

  } else if (command == "merge") {

    string rev_to_merge{argv[2]};

    for (const auto &entry : fs::directory_iterator(lit_dir / rev_to_merge)) {
      string filename = entry.path().filename().u8string();
      string base_file = lit_dir / head_dir / filename;

      if (regex_match(filename, RX_COMMIT_FILES)) {
        continue;
      }
      if (!fs::exists(base_file)) {
        rcopy_file(entry.path(), cwd);
      } else {
        // TODO merging with conflicts
      }
    }

    branch branch{cwd};
    branch.merge_branch(rev_to_merge);

    string message = "merge " + rev_to_merge + " into " + head_dir;
    commit merge_commit{cwd, message};
    merge_commit.create_merge_commit(rev_to_merge);

  } else if (command == "log") {

    commit_graph graph{cwd};
    graph.print_graph();
  } else {
    return print_error("command not known.");
  }

  return 0;
}
