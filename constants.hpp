#ifndef CONST_HPP
#define CONST_HPP

#include <string>

const std::string LIT_DIR = ".lit";
const std::string LITIGNORE = ".litignore";
const std::string REVISION_PX = "r";
const std::string COMMIT_ID_FILE = "commit.id";
const std::string COMMIT_PARENT_ID_FILE = "commit.parent.id";
const std::string COMMIT_INFO_FILE = "commit.info";
const std::string COMMIT_MESSAGE_FILE = "commit.message";
const std::string COMMIT_HEAD = "commit.head";
const std::string COMMIT_FILE_PATTERN = "commit.*";

const std::string CMD_INIT = "init";
const std::string CMD_STATUS = "status";
const std::string CMD_COMMIT = "commit";
const std::string CMD_MERGE = "merge";
const std::string CMD_CHECKOUT = "checkout";
const std::string CMD_SHOW = "show";
const std::string CMD_LOG = "log";
const std::string CMD_HELP = "help";

#endif /* CONST_HPP */