#ifndef BRANCH_HPP
#define BRANCH_HPP

#include <filesystem>
#include <set>
#include <string>

class branch {

private:
  std::filesystem::path branch_file;
  std::set<std::string> branches;

  void serialize();

public:
  branch(std::filesystem::path cwd);
  bool is_branch(const std::string &revision);
  void update_branch(const std::string &old_revision,
                     const std::string &new_revision);
  void add_branch(const std::string &revision);
  std::set<std::string> get_active_branches();
  void merge_branch(const std::string &to_merge);
};

#endif /* BRANCH_HPP */
