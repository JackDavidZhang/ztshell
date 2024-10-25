#include<pwd.h>
#include<sstream>

#ifndef BUILD_IN_H
#define BUILD_IN_H
extern bool runningFlag;
extern passwd* pwd;
extern std::string path;
extern std::string scPath;
int shellExit();
int shellLs(std::istringstream &istr);
int shellCd(std::istringstream &istr);
int shellPwd(std::istringstream &istr);

#endif