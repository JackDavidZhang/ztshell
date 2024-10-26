#include "../include/env.hpp"

#include <cstdint>
#include <iostream>
#include<map>
#include <stdexcept>

std::map<uint64_t, std::string> vars;
std::map<uint64_t, std::string> names;

void set_env(const std::string *key, const std::string *value) {
    uint64_t key_hash = std::hash<std::string>()(*key);
    vars.insert(std::pair<uint64_t, std::string>(key_hash, *value));
    names.insert(std::pair<uint64_t, std::string>(key_hash, *key));
}

std::string get_env(const std::string *key) {
    uint64_t key_hash = std::hash<std::string>()(*key);
    std::string var;
    try {
        var = vars.at(key_hash);
    } catch (std::out_of_range &e) {
        var = "";
    }
    return var;
}

void print_env() {
    for (const auto &var: vars) {
        std::cout << names[var.first] << '=' << var.second << std::endl;
    }
}
