#include <filesystem>
#include <fstream>
#include <iostream>
#include <regex>
#include <string>

#include "headers/file_utils.hpp"

namespace fs = std::filesystem;
using std::string;

int read_int_from_file(fs::path file_path) {
  int num = 0;
  std::ifstream file;
  file.open(file_path);
  file >> num;
  return num;
}

std::vector<int> read_vec_from_file(fs::path file_path) {
  std::string line;
  std::ifstream file;
  std::vector<int> ids;
  file.open(file_path);
  while (getline(file, line)) {
    ids.push_back(std::stoi(line));
  }
  return ids;
}

string read_string_from_file(fs::path file_path) {
  std::ifstream file;
  file.open(file_path);
  std::stringstream ss;
  ss << file.rdbuf();
  return ss.str();
}

void write_to_file(fs::path dir, const string &str) {
  std::ofstream commit_file_out;
  commit_file_out.open(dir);
  commit_file_out << str;
}

void write_to_file(fs::path dir, std::vector<int> numbers) {
  std::ofstream commit_file_out;
  commit_file_out.open(dir, std::ios_base::app);
  commit_file_out << std::to_string(numbers.at(0));
  for (int i = 1; i < numbers.size(); i++) {
    commit_file_out << std::endl;
    commit_file_out << std::to_string(numbers.at(i));
  }
}

void write_to_file(fs::path dir, const int number) {
  write_to_file(dir, std::to_string(number));
}

void delete_files(fs::path dir, std::regex exclude) {

  for (const auto &entry : fs::directory_iterator(dir)) {
    string filename = entry.path().filename().u8string();
    if (!regex_match(filename, exclude)) {
      fs::remove_all(entry.path());
    }
  }
}

void copy_files_exclude(fs::path from, fs::path to, std::regex exclude,
                        bool override) {

  if (override) {
    delete_files(to, exclude);
  }

  for (const auto &entry : fs::directory_iterator(from)) {
    string filename = entry.path().filename().u8string();
    if (!regex_match(filename, exclude)) {
      fs::copy(entry.path(), to);
    }
  }
}

void copy_files(fs::path from, fs::path to) {
  std::regex exclude("");
  copy_files_exclude(from, to, exclude, false);
}

bool compare_directories(fs::path from, fs::path to, std::regex exclude,
                         std::string identifier) {
  bool new_files = false;
  for (const auto &entry : fs::directory_iterator(from)) {
    string filename = entry.path().filename().u8string();
    if (!regex_match(filename, exclude)) {
      if (!fs::exists(to / filename)) {
        if (!identifier.empty())
          std::cout << identifier << ": " << filename << std::endl;
        new_files = true;
      }
    }
  }
  return new_files;
}
