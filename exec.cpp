#include <iostream>
#include <string>

#include "exec.hpp"

namespace fs = std::filesystem;
using std::string;

exec::exec(string command_str, std::vector<string> arg_vec) {

  string args = "";
  for (auto arg : arg_vec) {
    args += " " + arg;
  }

  command = command_str + args;
}

string exec::run() {

  std::array<char, 128> buffer;
  string result;

  FILE *pipe = popen(command.c_str(), "r");
  if (!pipe) {
    return result;
  }

  while (fgets(buffer.data(), 128, pipe) != NULL) {
    result += buffer.data();
  }
  pclose(pipe);

  return result;
}
