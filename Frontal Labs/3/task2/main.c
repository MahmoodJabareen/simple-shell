#include "util.h"
#define SYS_WRITE 4
#define STDOUT 1
extern void infection();
extern void infector(char* filename);
extern int system_call();

int main(int argc, char** argv, char* envp[]) {
    if (argc != 2 ||  argv[1][0] != '-' || argv[1][1] != 'a') {
        return 0x55; // Error code
    }
    char* filename = argv[1] + 2;
    int len = strlen(filename);
    system_call(SYS_WRITE,STDOUT,filename,len) ;
    system_call(SYS_WRITE,STDOUT,"\n",1) ;
    infection();
    infector(filename);
    return 0;
}