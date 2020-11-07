#ifndef EXEC_HPP
#define EXEC_HPP

#include <filesystem>
#include <string>
#include <vector>

class exec {

private:
  std::string command;

public:
  exec(std::string, std::vector<std::string>);
  std::string run();
};

#endif /* EXEC_HPP */