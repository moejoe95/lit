#include <array>
#include <chrono>
#include <ctime>
#include <fstream>
#include <iostream>
#include <regex>
#include <string>

#include "branch.hpp"
#include "commit.hpp"
#include "constants.hpp"
#include "exec.hpp"
#include "file_utils.hpp"

namespace fs = std::filesystem;
using std::string;

commit::commit(fs::path cwd_path, const string message)
    : cwd_path(cwd_path), message(message) {

  lit_path = cwd_path / LIT_DIR;
  timestamp = std::chrono::system_clock::now();

  if (!fs::exists(LIT_DIR)) {
    parent_id = 0;
    id = 1;
    revision_dir = lit_path / "r1";
    parent_revision_dir = lit_path / "r0";
    init_commit();
  } else {
    parent_id = read_int_from_file(lit_path / COMMIT_HEAD);
    id = read_int_from_file(lit_path / COMMIT_ID_FILE) + 1;
    revision_dir = lit_path / get_revision_name();
    parent_revision_dir = lit_path / get_parent_revision_name();
  }
  // update branch
  branch branch{cwd_path};
  if (branch.is_branch(get_parent_revision_name())) {
    branch.update_branch(get_parent_revision_name(), get_revision_name());
  } else {
    branch.add_branch(get_revision_name());
  }
}

void commit::init_commit() {
  // .litignore file
  std::stringstream litignore;
  litignore << LIT_DIR << std::endl << COMMIT_FILE_PATTERN;
  write_to_file(cwd_path / LITIGNORE, litignore.str());

  fs::create_directories(lit_path);

  // create branches file
  write_to_file(lit_path / COMMIT_BRANCHES, "r1");

  write_commit_infos();

  print_commit();
}

void commit::create_commit() {

  string out = diff();

  if (out.empty()) {
    std::cout << "nothing to commit" << std::endl;
    return;
  }

  std::regex exclude(COMMIT_FILE_PATTERN);
  delete_files(parent_revision_dir, exclude);

  fs::create_directories(revision_dir);
  fs::path patch_path = revision_dir / COMMIT_PATCH;
  write_to_file(patch_path, out);

  write_commit_infos();

  print_commit();
}

void commit::write_commit_infos() {
  // create/update commit-state files
  write_to_file(lit_path / COMMIT_ID_FILE, id);
  write_to_file(lit_path / COMMIT_HEAD, id);

  // copy to new revision directory
  if (!fs::exists(revision_dir)) {
    fs::create_directories(revision_dir);
  }

  std::regex exclude(".lit");
  copy_files_exclude(cwd_path, revision_dir, exclude, false);

  // write commit.message file
  write_to_file(revision_dir / COMMIT_MESSAGE_FILE, message);
  write_to_file(revision_dir / COMMIT_PARENT_ID_FILE, parent_id);
}

string commit::diff() {
  string litignore_file = cwd_path / LITIGNORE;

  std::vector<string> args{"-ruN", cwd_path, parent_revision_dir,
                           "--exclude-from=" + litignore_file};
  exec cmd("diff", args);

  return cmd.run();
}

void commit::print_commit() {

  using namespace std::chrono;

  std::stringstream commit_infos;
  commit_infos << "Commit: " << get_revision_name() << std::endl;

  milliseconds ms = duration_cast<milliseconds>(timestamp.time_since_epoch());
  seconds s = duration_cast<seconds>(ms);
  std::time_t t = s.count();

  commit_infos << "Date: " << std::ctime(&t) << std::endl;

  commit_infos << message << std::endl << std::endl;

  std::cout << commit_infos.str();

  write_to_file(revision_dir / COMMIT_INFO_FILE, commit_infos.str());
}

string commit::get_revision_name() { return REVISION_PX + std::to_string(id); }

string commit::get_parent_revision_name() {
  return REVISION_PX + std::to_string(parent_id);
}
