#include<iostream>

int main(int argc, char *argv[]) {
    if(argc == 1){
        char c;
        while((c=getchar())!=EOF)
            putchar(c);
    }
    if(argc == 2) {
        FILE* f = fopen(argv[1], "r");
        if(f!=nullptr) {
            char c;
            while((c=fgetc(f))!=EOF) putchar(c);
        }else{
            std::cerr << "Cannot open file " << argv[1] << std::endl;
        }
    }
    return 0;
}