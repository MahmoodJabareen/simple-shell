#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
int digit_counter(char *number)
{
    int counter = 0;
    for (size_t i = 0; number[i] != '\0'; i++)
    {
        if (number[i] <= '9' && number[i] >= '0')
            counter++;
    }
    return counter;
}

int main(int argc, char **argv)
{
    if (argc < 2)
    {
        printf("Error you have not put a number ");
        exit(0);
    }
    printf("num of digits %d :\n", digit_counter(argv[1]));
}


