#include "headers/checkout.hpp"
#include "headers/branch.hpp"
#include "headers/constants.hpp"
#include "headers/exec.hpp"
#include "headers/file_utils.hpp"
#include <iostream>
#include <string>

using std::cout;
using std::endl;
using std::string;
namespace fs = std::filesystem;

checkout::checkout(fs::path cwd, fs::path head_dir, fs::path to_checkout)
    : cwd(cwd), head_dir(head_dir), to_checkout(to_checkout) {
  lit_dir = cwd / LIT_DIR;
}

void checkout::create(string rev_dir, int head_id) {
  branch branch{cwd};
  std::set<string> branches = branch.get_active_branches();

  if (branches.count(head_dir) < 1) {
    delete_files(lit_dir / head_dir, RX_COMMIT_FILES);
  }

  bool rev_found = false;
  for (auto b : branches) {
    copy_files_exclude(lit_dir / b, cwd, RX_LIT_COMMIT_FILES, true);

    if (to_checkout == b) {
      int branch_id = std::stoi(b.substr(1, b.size()));
      write_to_file(lit_dir / COMMIT_HEAD, branch_id);
      break;
    }
    rev_dir = lit_checkout(b);
    if (rev_dir == to_checkout) {
      rev_found = true;
      break;
    }
  }

  if (rev_dir == "r-1") {
    cout << "revision: " << to_checkout << " not found" << endl;
    write_to_file(lit_dir / COMMIT_HEAD, head_id);
  }

  // copy files needed if new branch created in next step
  if (rev_found) {
    copy_files_exclude(cwd, lit_dir / to_checkout, RX_LIT_FILES, false);
  }
}

// PRIVATE HELPERS

void checkout::patch(fs::path dir, fs::path patch_file) {
  std::vector<string> args{"-ruN", "-d", dir, "<", patch_file};
  exec cmd("patch", args);
  cmd.run();
}

string checkout::lit_checkout_next(const string &rev, int parent_id) {
  fs::path patch_file = lit_dir / rev / COMMIT_PATCH;

  // patch directories
  for (const auto &entry : fs::directory_iterator(cwd)) {
    string filename = entry.path().filename().u8string();
    if (entry.is_directory()) {
      patch(cwd / filename, patch_file);
    }
  }

  // patch files
  patch(cwd, patch_file);

  // reset head
  write_to_file(lit_dir / COMMIT_HEAD, parent_id);

  string parent_rev_dir = REVISION_PX + std::to_string(parent_id);
  return parent_rev_dir;
}

string checkout::lit_checkout(string rev_dir) {

  fs::path lit_dir = cwd / LIT_DIR;
  std::vector<int> parent_ids =
      read_vec_from_file(lit_dir / rev_dir / COMMIT_PARENT_ID_FILE);

  for (int p : parent_ids) {
    rev_dir = lit_checkout_next(rev_dir, p);
    if (rev_dir == "r0" || to_checkout == rev_dir) {
      return rev_dir;
    } else {
      rev_dir = lit_checkout(rev_dir);
    }
  }

  return rev_dir;
}
