#include "../include/env.hpp"

#include <cstdint>
#include <iostream>
#include <map>
#include <stdexcept>
#include <thread>

std::map<uint64_t, std::string> vars;
std::map<uint64_t, std::string> names;

void env_init() {
    vars.clear();
    names.clear();
    char **sysenvp = environ;
    while (*sysenvp != NULL) {
        std::string name;
        std::string value;
        for (int i = 0; sysenvp[0][i]; i++) {
            if (sysenvp[0][i] == '=') {
                name = std::string(sysenvp[0]).substr(0, i);
                value = std::string(sysenvp[0]).substr(i + 1);
                break;
            }
        }
        set_env(&name, &value);
        sysenvp++;
    }
    uint64_t path_hash = std::hash<std::string>()("PATH");
    vars[path_hash] = ".:" + vars[path_hash];
}

void set_env(const std::string *key, const std::string *value) {
    uint64_t key_hash = std::hash<std::string>()(*key);
    if (vars.find(key_hash) == vars.end()) {
        vars.insert(std::pair<uint64_t, std::string>(key_hash, *value));
        names.insert(std::pair<uint64_t, std::string>(key_hash, *key));
    } else {
        vars[key_hash] = *value;
        names[key_hash] = *key;
    }
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
