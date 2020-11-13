#ifndef FILE_UTILS_HPP
#define FILE_UTILS_HPP

#include <filesystem>
#include <regex>
#include <string>
#include <vector>

int read_int_from_file(std::filesystem::path dir);
std::vector<int> read_vec_from_file(std::filesystem::path file_path);
std::string read_string_from_file(std::filesystem::path dir);
void write_to_file(std::filesystem::path file, const std::string &str);
void write_to_file(std::filesystem::path file, const int number);
void write_to_file(std::filesystem::path dir, std::vector<int> numbers);
void delete_files(std::filesystem::path from, std::regex exclude);
void copy_files_exclude(std::filesystem::path from, std::filesystem::path to,
                        std::regex exclude, bool override);
void copy_files(std::filesystem::path from, std::filesystem::path to);
void crop_file_names(std::filesystem::path dir, std::string file_ends);
bool compare_directories(std::filesystem::path from, std::filesystem::path to,
                         std::regex exclude, std::string identifier);

#endif /* FILE_UTILS_HPP */
