#ifndef COMMIT_HPP
#define COMMIT_HPP

#include <chrono>
#include <filesystem>
#include <string>

class commit {

private:
  int id;
  int parent_id;
  bool init;
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
  commit(std::filesystem::path cwd, std::string message);
  void create_commit();
};

#endif /* COMMIT_HPP */
