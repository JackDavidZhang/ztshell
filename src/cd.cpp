#include<iostream>
#include<sstream>
#include <unistd.h>

#include "../include/build_in.hpp"

int shellCd(std::istringstream &istr) {
    std::string cpath;
    istr >> cpath;
    if (!istr.eof()) {
        std::cout << "Too many arguments." << std::endl;
        return -1;
    }
    if (chdir(cpath.c_str())) {
        std::cout << "Can't change working directory to " << cpath << std::endl;
        return -1;
    } else {
        path = getcwd(nullptr, 0);
        if ((path == pwd->pw_dir) || path.substr(0, std::string(pwd->pw_dir).size() + 1) == std::string(pwd->pw_dir) +
            "/") {
            scPath = "~" + path.substr(std::string(pwd->pw_dir).size());
        } else {
            scPath = path;
        }
        return 0;
    }
}
