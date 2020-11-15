#ifndef CONST_HPP
#define CONST_HPP

#include <regex>
#include <string>

const std::string LIT_DIR = ".lit";
const std::string LITIGNORE = ".litignore";
const std::string REVISION_PX = "r";
const std::string COMMIT_ID_FILE = "commit.id";
const std::string COMMIT_PARENT_ID_FILE = "commit.parent.id";
const std::string COMMIT_INFO_FILE = "commit.info";
const std::string COMMIT_MESSAGE_FILE = "commit.message";
const std::string COMMIT_HEAD = "commit.head";
const std::string COMMIT_PATCH = "commit.patch";
const std::string COMMIT_BRANCHES = "commit.branches";
const std::string COMMIT_FILE_PATTERN = "commit.*";

const std::regex RX_COMMIT_FILES("(^commit.*)");
const std::regex RX_LIT_COMMIT_FILES("(^.lit*|^commit.*)");
const std::regex RX_LIT_FILES("(^.lit*)");
const std::regex RX_NO("$^"); // matches nothing

#endif /* CONST_HPP */