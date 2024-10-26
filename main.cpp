#include <iostream>
#include <thread>
#include <vector>
#include <readline/readline.h>
#include <csignal>
#include <sys/utsname.h>
#include <pwd.h>
#include <dirent.h>
#include "include/build_in.hpp"
#include "include/env.hpp"

void mainThread();

void sigHandler(int sig);

int execCommand(arguments &);

int shellEcho(arguments &);

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
    std::cout << timeinfo->tm_hour << ":";
    if (timeinfo->tm_min < 10)
        std::cout << 0;
    std::cout << timeinfo->tm_min << ":";
    if (timeinfo->tm_sec < 10)
        std::cout << 0;
    std::cout << timeinfo->tm_sec << ' ' << pwd->pw_name << '@'
            << un.nodename << ' ' << scPath << ' ';
    input = readline(" > ");
    if (input == NULL) {
        shellExit();
    } else {
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
                } else {
                    if (blank) {
                        words.emplace_back("");
                        blank = false;
                    }
                    words.back() += input[i];
                }
            }
            if (input[i] == '"') quoted = !quoted;
        }
        free(input);
        if (quoted) {
            std::cout << "wrong format" << std::endl;
            execStatus = -1;
            return;
        }
        if (words.empty()) {
            execStatus = 0;
            return;
        }
        for (int i = 0; i < words.size(); i++) {
            if (words[i][0] == '"' && words[i][words[i].size() - 1] == '"')
                words[i] = words[i].substr(1, words[i].size() - 2);
            else {
                if (words[i] == "~" || words[i].substr(0, 2) == "~/") {
                    words[i] = pwd->pw_dir + words[i].substr(1);
                }
                bool matched = false;
                int begin = -1, length = -1;
                for (int j = 0; j < words[i].size(); j++) {
                    if (matched) {
                        if ((words[i][j] <= 'z' && words[i][j] >= 'a') || (words[i][j] >= 'A' && words[i][j] <= 'Z') ||
                            (words[i][j] >= '0' && words[i][j] <= '9') || words[i][j] == '_') {
                            length++;
                        } else {
                            matched = false;
                            if (length > 1) {
                                if ((words[i][begin + 1] >= 'a' && words[i][begin + 1] <= 'z') || (
                                        words[i][begin + 1] >= 'A' && words[i][begin + 1] <= 'Z')) {
                                    std::string var = words[i].substr(begin+1,length-1);
                                    std::string value = get_env(&var);
                                    words[i] = words[i].substr(0,begin)+value+words[i].substr(begin+length);
                                    j=begin+value.size();
                                }
                            }
                            j--;
                        }
                    } else {
                        if (words[i][j] == '$') {
                            matched = true;
                            begin = j;
                            length = 1;
                        }
                    }
                }
                if (matched && ((words[i][begin + 1] >= 'a' && words[i][begin + 1] <= 'z') || (
                words[i][begin + 1] >= 'A' && words[i][begin + 1] <= 'Z'))) {
                    std::string var = words[i].substr(begin+1,length-1);
                    std::string value = get_env(&var);
                    words[i] = words[i].substr(0,begin)+value+words[i].substr(begin+length);
                }
            }
        }
        arguments arg;
        arg.argc = words.size()-1;
        arg.argv = words.empty() ? nullptr : &words[1];
        arg.command = words[0];
        execStatus = execCommand(arg);
    }
}

int execCommand(arguments &arg) {
    if (arg.command.empty()) return 0;

    if (arg.command == "exit") {
        return shellExit();
    }
    if (arg.command == "echo") {
        return shellEcho(arg);
    }
    if (arg.command == "ls") {
        return shellLs(arg);
    }
    if (arg.command == "pwd") {
        return shellPwd(arg);
    }
    if (arg.command == "cd") {
        return shellCd(arg);
    }
    if(arg.command == "export") {
        return shellExport(arg);
    }
    std::cout << "ztsh: command not found: " << arg.command << std::endl;
    return -1;
}

int shellEcho(arguments& arg) {
    for(int i = 0; i < arg.argc; i++) {
        std::cout << arg.argv[i] << " ";
    }
    std::cout << std::endl;
    return 0;
}

void sigHandler(int sig) {
    if (inThread) pthread_cancel(main_thread.native_handle());
    std::cout << sig << std::endl;
    execStatus = -1;
}
