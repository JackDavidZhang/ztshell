#include<iostream>
#include <unistd.h>

#include "../include/build_in.hpp"

int shellCd(const arguments &arg) {
    if (arg.argc > 1) {
        std::cout << "Too many arguments." << std::endl;
        return -1;
    }
    if (arg.argc == 0) {
        std::cout << "Too few arguments." << std::endl;
        return -1;
    }
    if (chdir(arg.argv[0].c_str())) {
        std::cout << "Can't change working directory to " << arg.argv[0] << std::endl;
        return -1;
    } else {
        char *cpath = getcwd(nullptr, 0);
        path = cpath;
        free(cpath);
        if ((path == pwd->pw_dir) || path.substr(0, std::string(pwd->pw_dir).size() + 1) == std::string(pwd->pw_dir) +
            "/") {
            scPath = "~" + path.substr(std::string(pwd->pw_dir).size());
        } else {
            scPath = path;
        }
        return 0;
    }
}
