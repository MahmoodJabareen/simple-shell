#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct fun_desc {
    char *name;
    char (*fun)(char);
};

char* map(char *array, int array_length, char (*f)(char)) {
    char* mapped_array = (char*)(malloc(array_length * sizeof(char)));
    for (int i = 0; i < array_length; i++) {
        mapped_array[i] = f(array[i]);
    }
    return mapped_array;
}

void my_get(char *carray, int array_length) {
    printf("Enter a string (up to %d characters): ", array_length);
    if (fgets(carray, array_length + 1, stdin) != NULL) {
        carray[strcspn(carray, "\n")] = '\0';
    }
    int c;
    while ((c = getchar()) != '\n' && c != EOF);
}


char cprt(char c) {
    if ((c >= 0x20) && (c < 0x7E)) {
        printf("%c\n", c);
    } else {
        printf(".\n");
    }
    return c;
}

char xport(char c) {
    printf("hexa : %02X\n", (unsigned char)c);
    printf("octal : %0o\n", (unsigned char)c);
    return c;
}
char encrypt(char c) {
    if (c >= 0x20 && c <= 0x7E) {
        return c + 0x03;
    }
    return c;
}

char decrypt(char c) {
    if (c >= 0x20 && c <= 0x7E) {
        return c - 0x03;
    }
    return c;
}

int main() {
    struct fun_desc menu[] = {
        {"Get String", my_get},
        {"Print String", cprt},
        {"Print Hex", xport},
        {"Encrypt", encrypt},
        {"Decrypt", decrypt},
        {NULL, NULL}
    };

    int menu_length = sizeof(menu) / sizeof(struct fun_desc) - 1;
    char *carray = calloc(5, sizeof(char));
    int carray_length = 5;

    while (1) {
        printf("Select operation from the following menu:\n\n");
        for (int i = 0; i < menu_length; i++) {
            if (menu[i].name != NULL) {
                printf("%d) %s\n", i, menu[i].name);
            }
        }

        printf("Option: ");
        char input[10];
        if (fgets(input, sizeof(input), stdin) == NULL) {
            break;
        }

        int option = atoi(input);
        if (option >= 0 && option < menu_length) {
            printf("Within bounds\n\n");
            if (option == 0) { // my_get
                my_get(carray, carray_length);
            } else {
                char *new_carray = map(carray, carray_length, menu[option].fun);
                strncpy(carray, new_carray, carray_length);
                free(new_carray);
            }
        } else {
            printf("Not within bounds\n\n");
            break;
        }
    }

    free(carray);
    return 0;
}
