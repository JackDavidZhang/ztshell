#include <cstdint>
#include <iostream>
#include<map>
#include <cstdint>

#include "../include/build_in.hpp"

int shellWhere(arguments &arg) {
    if (arg.argc > 1) {
        std::cout << "Too many arguments." << std::endl;
        return -1;
    } else if (arg.argc != 1) {
        std::cout << "Wrong arguments." << std::endl;
    }
    uint64_t commandHash = std::hash<std::string>()(arg.argv[0]);
    if (execMap.find(commandHash) == execMap.end()) {
        std::cout << "Command not found." << std::endl;
        return -1;
    } else {
        std::cout << execMap[commandHash] << std::endl;
        return 0;
    }
}
