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
    pid_t pid = 0;
    int in = 0;
    int out = 1;
    int err = 2;
};
int shellExit();
int shellCd(const arguments& arg);
int shellPwd(const arguments& arg);
int shellExport(arguments& arg);
int shellWhere(arguments& arg);

#endif
