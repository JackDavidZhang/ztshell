#include <iostream>
#include <thread>
#include <sstream>
#include <vector>
#include <readline/readline.h>
#include <csignal>
#include <sys/utsname.h>
#include <pwd.h>
#include <dirent.h>

void mainThread();

int shellExit();

void sigHandler(int sig);

int execCommand(std::istringstream &);

int shellEcho(std::istringstream &);

int shellLs(std::istringstream &);

int shellPwd(std::istringstream &);

int shellCd(std::istringstream &);

bool flag = true;
bool inThread = false;
std::string path;
std::string scPath;
std::thread main_thread;
utsname un;
passwd *pwd;
tm *timeinfo;

int execStatus = 0;

int main() {
    struct sigaction act;
    act.sa_handler = sigHandler;
    sigaction(SIGINT, &act, nullptr);
    inThread = true;
    if (uname(&un)) {
        return 1;
    }
    pwd = getpwuid(getuid());
    path = pwd->pw_dir;
    chdir(path.c_str());
    scPath = "~";
    while (flag) {
        main_thread = std::thread(mainThread);
        main_thread.join();
    }
    inThread = false;
    return 0;
}

void mainThread() {
    char *input;
    if (execStatus) std::cout << "\33[31mx\33[0m ";
    time_t now = time(nullptr);
    timeinfo = localtime(&now);
    std::cout << timeinfo->tm_hour << ":" << timeinfo->tm_min << ":" << timeinfo->tm_sec << ' ' << pwd->pw_name << '@'
            << un.nodename << ' ' << scPath << ' ';
    input = readline(" > ");
    if (input == NULL) {
        shellExit();
    } else {
        std::istringstream istr(input);
        free(input);
        execStatus = execCommand(istr);
    }
}

int execCommand(std::istringstream &istr) {
    std::string command;
    if (!(istr >> command)) return 0;
    if (command == "exit") {
        return shellExit();
    }
    if (command == "echo") {
        return shellEcho(istr);
    }
    if (command == "ls") {
        return shellLs(istr);
    }
    if (command == "pwd") {
        return shellPwd(istr);
    }
    if (command == "cd") {
        return shellCd(istr);
    }
    std::cout << "ztsh: command not found: " << command << std::endl;
    return -1;
}

int shellExit() {
    flag = false;
    return 0;
}

int shellEcho(std::istringstream &istr) {
    std::vector<std::string> paragrams;
    std::string para;
    while (istr >> para) {
        std::cout << para << " ";
    }
    std::cout << std::endl;
    return 0;
}

void sigHandler(int sig) {
    if (inThread) pthread_cancel(main_thread.native_handle());
    std::cout << sig << std::endl;
    execStatus = -1;
}

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

int shellPwd(std::istringstream &istr) {
    if (istr.eof()) {
        char* wd = getcwd(nullptr, 0);
        std::cout << wd << std::endl;
        return 0;
    } else {
        std::cout << "Too many arguments." << std::endl;
        return -1;
    }
}

int shellCd(std::istringstream &istr) {
    std::string cpath;
    istr >> cpath;
    if (!istr.eof()) {
        std::cout << "Too many arguments." << std::endl;
        return -1;
    }
    if(chdir(cpath.c_str())) {
        std::cout << "Can't change working directory to " << cpath << std::endl;
        return -1;
    }else {
        path = getcwd(nullptr, 0);
        if((path==pwd->pw_dir)||path.substr(0,std::string(pwd->pw_dir).size()+1)==std::string(pwd->pw_dir)+"/") {
            scPath = "~"  + path.substr(std::string(pwd->pw_dir).size());
        }else {
            scPath = path;
        }
        return 0;
    }
}
