#include<pwd.h>
#include<string>
#include<map>
#include<cstdint>
#include<vector>
#ifndef BUILD_IN_H
#define BUILD_IN_H
extern bool runningFlag;
extern passwd* pwd;
extern std::string path;
extern std::string scPath;
extern std::map<uint64_t, std::string> execMap;
struct arguments {
    std::string command;
    int argc;
    std::string* argv;
    std::vector<int> stdinfds;
    std::vector<int> stdoutfds;
    std::vector<int> stderrfds;
    pid_t pid;
    int in;
    int out;
    int err;
};
int shellExit();
int shellCd(const arguments& arg);
int shellPwd(const arguments& arg);
int shellExport(arguments& arg);
int shellWhere(arguments& arg);

#endif