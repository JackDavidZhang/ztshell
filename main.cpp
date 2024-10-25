#include <iostream>
#include <thread>
#include <sstream>
#include <vector>
#include <readline/readline.h>
#include <csignal>
#include <sys/utsname.h>
#include <pwd.h>
#include <dirent.h>
#include "include/build_in.hpp"

void mainThread();

void sigHandler(int sig);

int execCommand(std::istringstream &);

int shellEcho(std::istringstream &);

bool inThread = false;
bool runningFlag = true;
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
    while (runningFlag) {
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
        std::vector<std::string> words;
        bool quoted = false;
        bool blank = true;
        for (int i = 0; i < strlen(input); i++) {
            if (quoted) {
                if (blank) {
                    words.emplace_back("");
                    blank = false;
                }
                words.back() += input[i];
            } else {
                if (input[i] == 9 || input[i] == 10 || input[i] == 13 || input[i] == 32) {
                    blank = true;
                } else if (blank) {
                    words.emplace_back("");
                    blank = false;
                }
                words.back() += input[i];
            }
            if (input[i] == '"') quoted = !quoted;
        }
        free(input);
        if (quoted) {
            std::cout << "wrong format" << std::endl;
            execStatus = -1;
            return;
        }
        if(words.empty()) {
            return;
        }
        for(int i = 0;i < words.size(); i++) {
            if(words[i][0]=='"'&&words[i][words[i].size()-1]=='"') words[i]=words[i].substr(1, words[i].size()-2);
            else {
                if(words[i]=="~"||words[i].substr(0,2)=="~/") {
                    words[i]=pwd->pw_dir+words[i].substr(1);
                }
                bool matched = false;
                int begin;
                for(int j = 0;j < words[i].size(); j++) {
                    if(words[i][j]=='$'){
                        matched = true;
                        begin = j;
                        continue;
                    }
                    if(matched) {
                        if((words[i][j]<='z'&&words[i][j]>='a')||(words[i][j]>='A'&&words[i][j]<='Z')||(words[i][j]>='1'&&words[i][j]<='9')||(words[i][j]>='0'&&words[i][j]=='_')) {
                            j++;
                        }else {
                            matched = false;

                        }
                    }
                }
            }
        }
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
