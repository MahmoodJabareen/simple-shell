#include <stdio.h>

int main(int argc, char *argv[]) {
    printf("Printing command-line arguments:\n");
    for (int i = 0; i < argc; ++i) {
        printf("arg[%d]: %s\n", i, argv[i]);
    }
    return 0;
}
