#include <iostream>

#include"../include/build_in.hpp"
#include"../include/env.hpp"

int shellExport(arguments& arg) {
    if (arg.argc > 1) {
        std::cout << "Too many arguments" << std::endl;
        return -1;
    }
    if (arg.argc == 1) {
        if (!((arg.argv[0][0] >= 'a' && arg.argv[0][0] <= 'z') || (arg.argv[0][0] >= 'A' && arg.argv[0][0] <= 'Z'))) {
            std::cout << "Invalid input" << std::endl;
            return -1;
        }
        int eq = -1;
        for (int i = 0; i < arg.argv[0].length(); i++) {
            if (arg.argv[0][i] == '=')
                if (eq < 0) eq = i;
                else {
                    std::cout << "format wrong" << std::endl;
                    return -1;
                }
            if (eq < 0 && !((arg.argv[0][i] >= 'a' && arg.argv[0][i] <= 'z') || (arg.argv[0][i] >= 'A' && arg.argv[0][i] <= 'Z') || (
                                arg.argv[0][i] >= '0' && arg.argv[0][i] <= '9') || (arg.argv[0][i] == '_'))) {
                std::cout << "Invalid input" << std::endl;
                return -1;
            }
        }
        if (eq < 0) {
            std::cout << "format wrong" << std::endl;
            return -1;
        } else {
            std::string name = arg.argv[0].substr(0, eq), value = arg.argv[0].substr(eq + 1);
            set_env(&name, &value);
            std::cout << name << '=' << value << std::endl;
        }
        return 0;
    }
    if (arg.argc == 0) {
        print_env();
        return 0;
    }
}
