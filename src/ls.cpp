#include <dirent.h>
#include <iostream>
#include"../include/build_in.hpp"

int shellLs(arguments &arg) {
    std::string target_path;
    if (arg.argc == 0)
        target_path = ".";
    else if (arg.argc == 1) {
        target_path = arg.argv[0];
    } else {
        std::cerr << "ls: Too many arguments\n";
        return -1;
    }
    DIR *dirp = opendir(target_path.c_str());
    if (dirp == NULL) {
        std::cout << "can not open directory " << target_path << std::endl;
        return -1;
    }
    std::cout << target_path << std::endl;
    struct dirent *entry;
    while ((entry = readdir(dirp))) {
        if (entry->d_name[0] == '.') continue;
        std::cout << entry->d_name << " ";
    }
    std::cout << std::endl;
    closedir(dirp);
    return 0;
}
