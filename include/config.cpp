#include "config.h"

using namespace std::chrono_literals;

GlobalConfig::GlobalConfig() {
    std::ifstream configFile("./config.txt", std::ios::in);
    while (!configFile.is_open()) {
        fprintf(stderr, "Failed to open config.txt! Retry after 1s.\n");
        std::this_thread::sleep_for(1000ms);
    }
    while (!configFile.eof()) {
        std::string node;
        while (true) {
            configFile >> node;
            Config config;
            if (node == "NODE1") {
                config.node = Config::Node::NODE1;
                configFile >> config.ip;
            } else if (node == "NODE2") {
                config.node = Config::Node::NODE2;
                configFile >> config.port;
            } else if (node == "###") {
                break;
            } else {
                NOT_REACHED
            }
            _config.push_back(config);
        }
    }
}

Config GlobalConfig::get(Config::Node node) {
    for (auto _i: _config) {
        if (_i.node == node) { return _i; }
    }
    NOT_REACHED
}
