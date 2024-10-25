#include <dirent.h>
#include<sstream>
#include<iostream>
#include"../include/build_in.hpp"

int shellLs(std::istringstream &istr) {
    DIR *dirp = opendir(path.c_str());
    if (dirp == NULL) {
        std::cout << "can not open directory " << path << std::endl;
        return -1;
    }
    std::cout << path << std::endl;
    struct dirent *entry;
    while ((entry = readdir(dirp))) {
        if (entry->d_name[0] == '.') continue;
        std::cout << entry->d_name << " ";
    }
    std::cout << std::endl;
    return 0;
}
