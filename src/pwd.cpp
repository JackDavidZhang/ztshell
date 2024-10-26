#include<iostream>
#include <unistd.h>
#include "../include/build_in.hpp"

int shellPwd(const arguments& arg) {
    if (arg.argc) {
        std::cout << "Too many arguments." << std::endl;
        return -1;
    } else {
        char *wd = getcwd(nullptr, 0);
        std::cout << wd << std::endl;
        return 0;
    }
}