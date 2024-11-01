#include <cstdint>
#include <iostream>
#include<map>
#include <cstdint>

#include "../include/build_in.hpp"

int shellWhere(arguments &arg) {
    if (arg.argc > 1) {
        std::cerr << "where: Too many arguments." << std::endl;
        return -1;
    } else if (arg.argc != 1) {
        std::cerr << "where: Wrong arguments." << std::endl;
        return -1;
    }
    if((arg.argv[0] == "cd")||(arg.argv[0] == "exit")||(arg.argv[0] == "export")||(arg.argv[0] == "pwd")||(arg.argv[0] == "where")) {
        std::cout << "built-in" << std::endl;
        return 0;
    }
    uint64_t commandHash = std::hash<std::string>()(arg.argv[0]);
    if (execMap.find(commandHash) == execMap.end()) {
        std::cerr << "where: Command not found." << std::endl;
        return -1;
    } else {
        std::cout << execMap[commandHash] << std::endl;
        return 0;
    }
}
