#include <complex>
#include <dirent.h>
#include <iostream>

int main(int argc, char *argv[]) {
    std::string target_path;
    if (argc == 1)
        target_path = ".";
    else if (argc == 2) {
        target_path = argv[1];
    } else {
        std::cerr << "ls: Too many arguments\n";
        return -1;
    }
    DIR *dirp = opendir(target_path.c_str());
    if (dirp == NULL) {
        std::cout << "ls: Can not open directory " << target_path << std::endl;
        return -1;
    }
    std::cout << "------------ LIST OF " << target_path << " ------------" <<std::endl;
    dirent *entry;
    while ((entry = readdir(dirp))) {
        if (entry->d_name[0] == '.') continue;
        std::cout << entry->d_name << " ";
    }
    std::cout << std::endl;
    closedir(dirp);
    return 0;
}
