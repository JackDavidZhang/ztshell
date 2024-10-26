#ifndef ENV_H
#define ENV_H

#include<string>

void set_env(const std::string*,const std::string*);
std::string get_env(const std::string*);
void print_env();

#endif //ENV_H
