#ifndef CONFIG_H
#define CONFIG_H

#include "utils.h"
#include <fstream>
#include <iostream>
#include <string>
#include <thread>
#include <vector>

class Config {
public:
    enum Node { NODE1 = 1, NODE2 = 2 };
    enum Type { FTP = 1, USER, PASS, PWD, CWD, PASV, LIST, RETR, BIN, BIN_END };

    std::string ip;
    int port;
    Node node;
};

class GlobalConfig {
public:
    GlobalConfig();
    Config get(Config::Node node);

private:
    std::vector<Config> _config;
};

#endif//CONFIG_H
