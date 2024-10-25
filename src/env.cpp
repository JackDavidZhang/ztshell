#include "../include/env.hpp"

#include <cstdint>
#include<map>
#include <stdexcept>

std::map<uint64_t,std::string> vars;

void set_env(const std::string* key,const std::string* value) {
    uint64_t key_hash = std::hash<std::string>()(*key);
    vars.insert(std::pair<uint64_t,std::string>(key_hash,*value));
}
std::string get_var(const std::string* key) {
    uint64_t key_hash = std::hash<std::string>()(*key);
    std::string var;
    try {
        var = vars.at(key_hash);
    }catch (std::out_of_range& e) {
        var = "";
    }
    return var;
}