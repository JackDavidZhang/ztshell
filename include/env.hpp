#ifndef ENV_H
#define ENV_H

#include <map>
#include<string>

//extern char **environ;

void env_init();
void set_env(const std::string*,const std::string*);
std::string get_env(const std::string*);
void print_env();
void readExecutable();

#endif //ENV_H
