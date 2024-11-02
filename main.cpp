#include <complex>
#include <csetjmp>
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
#include <map>
#include <queue>
#include <utility>
#include<sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>
#include <fcntl.h>

void mainCyc();

void sigHandler(int sig);

int execCommand(arguments &);

int shellEcho(arguments &);

bool runningFlag = true;
std::string path;
std::string scPath;
std::thread main_thread;
utsname un;
passwd *pwd;
tm *timeinfo;
jmp_buf mainCycle;
bool forked = false;
pid_t cpid;
std::map<uint64_t, std::string> execMap;
std::queue<int> openedfd;

int execStatus = 0;

int main() {
    struct sigaction act{};
    act.sa_handler = sigHandler;
    sigaction(SIGINT, &act, nullptr);
    if (uname(&un)) {
        return 1;
    }
    pwd = getpwuid(getuid());
    path = pwd->pw_dir;
    chdir(path.c_str());
    scPath = "~";
    env_init();
    readExecutable();
    while(!openedfd.empty()) {openedfd.pop();}
    setjmp(mainCycle);
    while (runningFlag) {
        mainCyc();
    }
    return 0;
}

void mainCyc() {
    while(!openedfd.empty()) {
        close(openedfd.front());
        openedfd.pop();
    }
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
    if (input == nullptr) {
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
        for (auto & word : words) {
            if (word[0] == '"' && word[word.size() - 1] == '"')
                word = word.substr(1, word.size() - 2);
            else {
                if (word == "~" || word.substr(0, 2) == "~/") {
                    word = pwd->pw_dir + word.substr(1);
                }
                bool matched = false;
                int begin = -1, length = -1;
                for (int j = 0; j < word.size(); j++) {
                    if (matched) {
                        if ((word[j] <= 'z' && word[j] >= 'a') || (word[j] >= 'A' && word[j] <= 'Z') ||
                            (word[j] >= '0' && word[j] <= '9') || word[j] == '_') {
                            length++;
                        } else {
                            matched = false;
                            if (length > 1) {
                                if ((word[begin + 1] >= 'a' && word[begin + 1] <= 'z') || (
                                        word[begin + 1] >= 'A' && word[begin + 1] <= 'Z') || (
                                        word[begin + 1] == '_')) {
                                    std::string var = word.substr(begin + 1, length - 1);
                                    std::string value = get_env(&var);
                                    word = word.substr(0, begin) + value + word.substr(begin + length);
                                    j = begin + value.size();
                                }
                            }
                            j--;
                        }
                    } else {
                        if (word[j] == '$') {
                            matched = true;
                            begin = j;
                            length = 1;
                        }
                    }
                }
                if (matched && ((word[begin + 1] >= 'a' && word[begin + 1] <= 'z') || (
                                    word[begin + 1] >= 'A' && word[begin + 1] <= 'Z') || (
                                    word[begin + 1] == '_'))) {
                    std::string var = word.substr(begin + 1, length - 1);
                    std::string value = get_env(&var);
                    word = word.substr(0, begin) + value + word.substr(begin + length);
                }
            }
        }
        int index = 0;
        std::vector<arguments> args ;
        if(words[words.size() - 1] == "|") {
            std::cerr << "Wrong format." << std::endl;
            execStatus = -1;
            return ;
        }
        words.push_back("|");
        int beg = 0;
        for(int i = 0; i < words.size(); i++) {
            if(words[i]=="|") {
                arguments arg;
                arg.argc = i-beg-1;
                if(arg.argc<0) {
                    std::cerr << "Wrong format." << std::endl;
                    execStatus = -1;
                    return ;
                }
                arg.command = words[beg];
                arg.argv = &words[beg+1];
                args.push_back(arg);
                beg = i+1;
            }
        }
        for(int i = 0; i < args.size(); i++) {
            int target = 0;
            for(int j = 0;j < args[i].argc; j++) {
                if(args[i].argv[j] == ">"||args[i].argv[j] == "0>"||args[i].argv[j] == "1>") {
                    if(j==args[i].argc-1) {
                        std::cerr << "Wrong format." << std::endl;
                        execStatus = -1;
                        return ;
                    }
                    int fd;
                    if((fd=open(args[i].argv[j+1].c_str(),O_WRONLY))==-1) {
                        std::cerr << "Cannot open file " << args[i].argv[j+1] << std::endl;
                        execStatus = -1;
                        return ;
                    }else {
                        openedfd.push(fd);
                        if(args[i].argv[j] == ">"||args[i].argv[j] == "0>")
                            args[i].stdoutfds.push_back(fd);
                        else args[i].stderrfds.push_back(fd);
                        args[i].argv[j+1].clear();
                    }
                    j++;
                    continue;
                }
                if(args[i].argv[j] == "<"||args[i].argv[j] == "2<") {
                    if(j==args[i].argc-1) {
                        std::cerr << "Wrong format." << std::endl;
                        execStatus = -1;
                        return ;
                    }
                    int fd;
                    if((fd=open(args[i].argv[j+1].c_str(),O_RDONLY))==-1) {
                        std::cerr << "Cannot open file " << args[i].argv[j+1] << std::endl;
                        execStatus = -1;
                        return ;
                    }else {
                        openedfd.push(fd);
                        args[i].stdinfds.push_back(fd);
                    }
                    j++;
                    continue;
                }
                args[i].argv[target] = args[i].argv[j];
                target++;
            }
            args[i].argc -= 2*(args[i].stderrfds.size()+args[i].stdoutfds.size()+args[i].stdinfds.size());
        }
        int fdin,fdout;
        for(int i = 0; i < args.size(); i++) {
            if(i) args[i].stdinfds.insert(args[i].stdinfds.begin(),fdin);
            if(i-args.size()+1) {
                int pipefd[2];
                if(pipe(pipefd) == -1) {
                    std::cerr << "Cannot create pipe." << std::endl;
                    execStatus = -1;
                    return ;
                }
                openedfd.push(pipefd[0]);
                openedfd.push(pipefd[1]);
                fdin =  pipefd[1];
                fdout = pipefd[0];
                args[i].stdoutfds.insert(args[i].stdoutfds.begin(),fdout);
            }
            execStatus = execCommand(args[i]);
        }}
}

int execCommand(arguments &arg) {
    if (arg.command.empty()) return 0;
    if (arg.command == "exit") {
        return shellExit();
    }
    if (arg.command == "pwd") {
        return shellPwd(arg);
    }
    if (arg.command == "cd") {
        return shellCd(arg);
    }
    if (arg.command == "export") {
        return shellExport(arg);
    }
    if (arg.command == "where") {
        return shellWhere(arg);
    }
    uint64_t commandHash = std::hash<std::string>()(arg.command);
    if (execMap.find(commandHash)!= execMap.end()) {
        std::cout << execMap[commandHash] << std::endl;
        pid_t pid;
        if((pid = fork())<0) return -1;
        else if(pid == 0) {
            std::vector <char*> args;
            args.clear();
            args.push_back(arg.command.data());
            for (int i = 0; i < arg.argc; i++) args.push_back(arg.argv[i].data());
            args.push_back(nullptr);
            execv(execMap[commandHash].c_str(), args.data());
        }else {
            int status;

            forked = true;
            cpid = pid;
            waitpid(pid,&status,0);
            forked = false;
            return status;
        }
        return 0;
    }
    std::cout << "ztsh: command not found: " << arg.command << std::endl;
    return -1;
}

int shellEcho(arguments &arg) {
    for (int i = 0; i < arg.argc; i++) {
        std::cout << arg.argv[i] << " ";
    }
    std::cout << std::endl;
    return 0;
}

void sigHandler(int sig) {
    if(forked) {
        kill(cpid,SIGINT);
        forked = false;
    }
    std::cout.flush();
    longjmp(mainCycle, 1);
}

void loadExecutable(std::string &path) {
    DIR *dirp = opendir(path.c_str());
    if (dirp == NULL) {
        std::cout << "can not open directory " << path << std::endl;
        return;
    }
    struct dirent *entry;
    auto *file = static_cast<struct stat *>(malloc(sizeof(struct stat)));
    while ((entry = readdir(dirp))) {
        if (entry->d_name[0] == '.') continue;
        if (stat((path + '/' + (entry->d_name)).c_str(), file)) {
            std::cout << "Error: " << entry->d_name << " " << errno << std::endl;
        } else {
            if (((file->st_mode) & S_IXUSR) && (S_ISREG(file->st_mode))) {
                uint64_t commandHash = std::hash<std::string>()(std::string(entry->d_name));
                if (execMap.find(commandHash) == execMap.end()) {
                    execMap.insert(std::pair<uint64_t, std::string>(commandHash, path + '/' + (entry->d_name)));
                } else {
                    execMap[commandHash] = path + '/' + (entry->d_name);
                }
            }
        }
    }
    free(file);
    closedir(dirp);
}

void readExecutable() {
    int beg = 0, end = 0;
    std::string spath = "PATH";
    std::string PATH = get_env(&spath);
    for (; PATH[end]; end++) {
        if (PATH[end] == ':') {
            std::string pPath = PATH.substr(beg, end - beg);
            loadExecutable(pPath);
            beg = end + 1;
        }
    }
    if (end - beg) {
        std::string pPath = PATH.substr(beg, end - beg);
        loadExecutable(pPath);
    }
}
