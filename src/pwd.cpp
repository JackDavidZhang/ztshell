#include <sstream>
#include<iostream>
#include <unistd.h>
#include "../include/build_in.hpp"

int shellPwd(std::istringstream &istr) {
    if (istr.eof()) {
        char *wd = getcwd(nullptr, 0);
        std::cout << wd << std::endl;
        return 0;
    } else {
        std::cout << "Too many arguments." << std::endl;
        return -1;
    }
}
