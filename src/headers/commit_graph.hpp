#ifndef COMMIT_GRAPH_HPP
#define COMMIT_GRAPH_HPP

#include "branch.hpp"
#include <filesystem>
#include <string>
#include <vector>

/* COMMIT_NODE */

class commit_node {

private:
  int id;
  std::string rev;
  std::filesystem::path lit_dir;
  std::vector<commit_node> parents;

public:
  commit_node(std::filesystem::path cwd, const std::string &id);
  void init_branch_graph();
  void print_node(std::vector<std::vector<std::string>> &grid, int col);
};

/* COMMIT_GRAPH */

class commit_graph {

private:
  std::vector<commit_node> start_nodes;
  std::filesystem::path cwd;
  std::vector<std::vector<std::string>> grid;
  int grid_rows;
  int grid_cols;

  void initialize_grid();
  void print_grid();

public:
  commit_graph(std::filesystem::path cwd);
  void print_graph();
};

#endif /* COMMIT_GRAPH_HPP */
