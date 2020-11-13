#include "headers/commit_graph.hpp"
#include "headers/constants.hpp"
#include "headers/file_utils.hpp"
#include <iostream>
#include <string>
#include <vector>

using std::cout;
using std::string;
using std::vector;
namespace fs = std::filesystem;

/* COMMIT_GRAPH IMPLEMENTATION */

commit_graph::commit_graph(fs::path cwd) : cwd(cwd) {
  branch branch{cwd};
  std::set<string> branches = branch.get_active_branches();

  for (auto b : branches) {
    commit_node node{cwd / LIT_DIR, b};
    node.init_branch_graph();
    start_nodes.push_back(node);
  }

  grid_rows = read_int_from_file(cwd / LIT_DIR / COMMIT_ID_FILE) + 1;
  grid_cols = 20; // TODO: dont hardcode this
  initialize_grid();
}

void commit_graph::initialize_grid() {
  for (int i = 0; i < grid_rows; i++) {
    vector<string> row;
    for (int j = 0; j < grid_cols; j++) {
      row.push_back(" ");
    }
    grid.push_back(row);
  }
}

void commit_graph::print_grid() {
  for (int i = grid_rows - 1; i >= 0; i--) {

    for (int j = 0; j < grid_cols; j++) {
      cout << grid[i][j];
    }

    string rev = REVISION_PX + std::to_string(i);
    // set arrow at head position
    int head_id = read_int_from_file(cwd / LIT_DIR / COMMIT_HEAD);
    if (i == head_id) {
      cout << "⇽ ";
    }
    cout << rev << ": "
         << read_string_from_file(cwd / LIT_DIR / rev / COMMIT_MESSAGE_FILE);
    cout << std::endl;
  }
}

void commit_graph::print_graph() {

  // fill grid
  int i = 0;
  for (auto n : start_nodes) {
    n.print_node(grid, i);
    i++;
  }

  // print grid
  print_grid();
}

/* COMMIT_NODE IMPLEMENTATION */

commit_node::commit_node(fs::path lit_dir, const string &rev)
    : rev(rev), lit_dir(lit_dir) {
  id = std::stoi(rev.substr(1, rev.size()));
}

void commit_node::init_branch_graph() {
  vector<int> parent_ids =
      read_vec_from_file(lit_dir / rev / COMMIT_PARENT_ID_FILE);

  for (int parent : parent_ids) {
    if (parent < 0)
      continue;
    string parent_rev = REVISION_PX + std::to_string(parent);
    commit_node parent_commit{lit_dir, parent_rev};
    parent_commit.init_branch_graph();
    parents.push_back(parent_commit);
  }
}

void commit_node::print_node(vector<vector<string>> &grid, int col) {

  // if field in grid is reached more than once, the commit is a new branch
  if (grid[id][col * 3] != " ") {
    grid[id][3] = "┘";
  }
  grid[id][col * 3] = "o";

  // set | in branches "before" current branch
  if (col > 0) {
    for (int i = 0; i < col * 3; i = i + 3) {
      grid[id][i] = "|";
    }
  }

  // if commit has more than one parent, it is a merge commit
  if (parents.size() > 1) {
    grid[id][3] = "┐";
  }

  int i = 0;
  for (auto p : parents) {
    p.print_node(grid, i); // print recursively
    i++;
  }
}
