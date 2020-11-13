#ifndef COMMIT_HPP
#define COMMIT_HPP

#include <chrono>
#include <filesystem>
#include <string>
#include <vector>

class commit {

private:
  int id;
  std::vector<int> parent_ids;
  std::filesystem::path cwd_path;
  std::filesystem::path lit_path;
  std::filesystem::path revision_dir;
  std::filesystem::path parent_revision_dir;
  std::string message;
  std::chrono::time_point<std::chrono::system_clock> timestamp;
  void print_commit();
  std::string get_parent_revision_name();
  std::string get_revision_name();
  std::string diff();
  void write_commit_infos();
  void init_commit();

public:
  commit(std::filesystem::path cwd, const std::string &message);
  commit(std::filesystem::path cwd);
  void create_commit();
  void create_merge_commit(const std::string &merge_rev);
};

#endif /* COMMIT_HPP */
