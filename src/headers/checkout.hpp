#ifndef CHECKOUT_HPP
#define CHECKOUT_HPP

#include <filesystem>
#include <set>
#include <string>

class checkout {

private:
  std::filesystem::path to_checkout;
  std::filesystem::path cwd;
  std::filesystem::path lit_dir;
  std::filesystem::path head_dir;

  void patch(std::filesystem::path dir, std::filesystem::path patch_file);
  std::string lit_checkout_next(const std::string &rev, int parent_id);
  std::string lit_checkout(std::string rev_dir);

public:
  checkout(std::filesystem::path cwd, std::filesystem::path head_dir,
           std::filesystem::path rev);
  void create(std::string rev_dir, int head_id);
};

#endif /* CHECKOUT_HPP */
