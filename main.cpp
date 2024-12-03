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
std::vector<pid_t> cpid;
std::map<uint64_t, std::string> execMap;
std::vector<arguments> args ;

int execStatus = 0;

int main() {
    struct sigaction act{};
    act.sa_handler = sigHandler;
    sigaction(SIGINT, &act, nullptr);
    if (uname(&un)) {
        return -1;
    }
    pwd = getpwuid(getuid());
    path = pwd->pw_dir;
    chdir(path.c_str());
    scPath = "~";
    env_init();
    readExecutable();
    setjmp(mainCycle);
    while (runningFlag) {
        mainCyc();
    }
    free(pwd);
    free(timeinfo);
    return 0;
}

void mainCyc() {
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
        args.clear();
        if(words[words.size() - 1] == "|") {
            std::cerr << "Wrong format." << std::endl;
            execStatus = -1;
            return ;
        }
        words.emplace_back("|");
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
                    if(i!=args.size()-1) {
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
                        if(args[i].argv[j] == ">"||args[i].argv[j] == "0>")
                            args[i].out=fd;
                        else args[i].err = fd;
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
                    if(i) {
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
                        args[i].in = fd;
                    }
                    j++;
                    continue;
                }
                args[i].argv[target] = args[i].argv[j];
                target++;
            }
            args[i].argc -= 2*(args[i].in!=0);
            args[i].argc -= 2*(args[i].out!=1);
            args[i].argc -= 2*(args[i].err!=2);
        }
        int pipefd[2]={0,1};
        for(int i = 0; i < args.size(); i++) {
            if(i) args[i].in = pipefd[0];
            if(i+1-args.size()) {
                if(pipe(pipefd) == -1) {
                    std::cerr << "Cannot create pipe." << std::endl;
                    execStatus = -1;
                    return ;
                }
                args[i].out = pipefd[1];
            }
            execStatus = execCommand(args[i]);
        }
        for(int i = 0; i < args.size(); i++) {
            int sta;
            waitpid(args[i].pid,&sta,0);
            if(!sta) execStatus = sta;
        }
    }
}

int execCommand(arguments &arg) {
//std::cout << arg.command << " " << arg.in << " " << arg.out <<  ' ' << arg.err << std::endl;
    if (arg.command.empty()) return 0;
    if(arg.command == "exit"||arg.command=="pwd"||arg.command=="cd"||arg.command=="export"||arg.command=="where") {
        int result = -1;
        if(arg.in != 0 ) stdin = fdopen(arg.in,"r");
        if(arg.out != 1 ) stdout = fdopen(arg.out,"w");
        if(arg.err != 2 ) stderr = fdopen(arg.err,"w");
        if (arg.command == "exit") {
            result = shellExit();
        }
        else if (arg.command == "pwd") {
            result = shellPwd(arg);
        }
        else if (arg.command == "cd") {
            result = shellCd(arg);
        }
        else if (arg.command == "export") {
            result = shellExport(arg);
        }
        else if (arg.command == "where") {
            result = shellWhere(arg);
        }
        if(arg.in != 0 ) stdin = fdopen(0,"r");
        if(arg.out != 1 ) stdout = fdopen(1,"w");
        if(arg.err != 2 ) stderr = fdopen(2,"w");
        return result;
    }
    uint64_t commandHash = std::hash<std::string>()(arg.command);
    if (execMap.find(commandHash)!= execMap.end()) {
        //std::cout << execMap[commandHash] << std::endl;
        pid_t pid;
        if((pid = fork())<0) return -1;
        else if(pid == 0) {
            std::vector <char*> args;
            args.clear();
            args.push_back(arg.command.data());
            for (int i = 0; i < arg.argc; i++) args.push_back(arg.argv[i].data());
            args.push_back(nullptr);
            dup2(arg.in,0);
            dup2(arg.out,1);
            dup2(arg.err,2);
            execv(execMap[commandHash].c_str(), args.data());
        }else {
            arg.pid = pid;
            if(arg.in!=0) close(arg.in);
            if(arg.out!=1) close(arg.out);
            if(arg.err!=2) close(arg.err);
            return 0;
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
    for(auto & arg : args) {
        if(arg.pid) {
            kill(arg.pid,SIGINT);
            waitpid(arg.pid,nullptr,0);
        }
    }
    fflush(stdout);
    fflush(stderr);
    //longjmp(mainCycle, 1);
}

void loadExecutable(std::string &path) {
    DIR *dirp = opendir(path.c_str());
    if (dirp == nullptr) {
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
