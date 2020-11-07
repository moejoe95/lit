#include <filesystem>
#include <fstream>
#include <iostream>
#include <set>
#include <string>

#include "branch.hpp"
#include "constants.hpp"

namespace fs = std::filesystem;

branch::branch(fs::path cwd) : branch_file(cwd / LIT_DIR / COMMIT_BRANCHES) {

  std::string line;
  std::ifstream file;
  file.open(branch_file);
  while (getline(file, line)) {
    branches.insert(line);
  }
}

bool branch::is_branch(const std::string &revision) {
  return branches.count(revision);
}

void branch::update_branch(const std::string &old_revision,
                           const std::string &new_revision) {
  branches.erase(old_revision);
  branches.insert(new_revision);
  serialize();
}

void branch::add_branch(const std::string &revision) {
  branches.insert(revision);
  serialize();
}

void branch::serialize() {
  fs::remove(branch_file);
  std::ofstream outfile;
  outfile.open(branch_file, std::ios_base::app);
  for (auto b : branches) {
    outfile << b << std::endl;
  }
}

std::set<std::string> branch::get_active_branches() { return branches; }
