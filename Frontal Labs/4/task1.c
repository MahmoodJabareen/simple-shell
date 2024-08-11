#include <stdlib.h>
#include <stdio.h>
#include "string.h"
typedef struct
{
    char debug_mode;
    char file_name[128];
    int unit_size;
    unsigned char mem_buf[10000];
    size_t mem_count;
    char display_mode;
    /*
     .
     .
     Any additional fields you deem necessary
    */
} state;

void ToggleDebugMode(state *s);
void SetFileName(state *s);
void SetUnitSize(state *s);
void LoadIntoMemory(state *s);
void ToggleDisplayMode(state *s);
void MemoryDisplay(state *s);
void SaveIntoFile(state *s);
void MemoryModify(state *s);
void Quit(state *s);
typedef struct func
{
    char *name;
    void (*function)(state *s);
} func;

struct func func_menu[] = {
    {"Toggle Debug Mode", ToggleDebugMode},
    {"Set File Name", SetFileName},
    {"Set Unit Size", SetUnitSize},
    {"Load Into Memory", LoadIntoMemory},
    {"Toggle Display Mode", ToggleDisplayMode},
    {"Memory Display", MemoryDisplay},
    {"Save Into File", SaveIntoFile},
    {"Memory Modify", MemoryModify},
    {"Quit", Quit},
    {NULL, NULL}};

void read_units_to_memory(FILE *fp, state *s, int length)
{
    fread(s->mem_buf, s->unit_size, length, fp);
    s->mem_count += length;
}
char *unit_to_decimal_format(int unit_size)
{
    static char *formats[] = {"%#hhd\n", "%#hd\n", "No such unit", "%#d\n"};
    return formats[unit_size - 1];
}
char *unit_to_hexadecimal_format(int unit_size)
{
    static char *formats[] = {"%#hhx\n", "%#hx\n", "No such unit", "%#x\n"};
    return formats[unit_size - 1];
}
void print_units_decimal(FILE *output, char *buffer, int count, state *s)
{
    int UnitSize = s->unit_size;
    char *end = buffer + UnitSize * count;
    while (buffer < end)
    {
        // print ints
        int var = *((int *)(buffer));
        fprintf(output, unit_to_decimal_format(UnitSize), var);
        buffer += UnitSize;
    }
}
void print_units_hexadecimal(FILE *output, char *buffer, int count, state *s)
{
    int UnitSize = s->unit_size;
    char *end = buffer + UnitSize * count;
    while (buffer < end)
    {
        // print ints
        int var = *((int *)(buffer));
        fprintf(output, unit_to_hexadecimal_format(UnitSize), var);
        buffer += UnitSize;
    }
}
void ToggleDebugMode(state *state)
{
    if (!state->debug_mode)
    {
        state->debug_mode = 1;
        printf("Debug flag now on\n");
    }
    else
    {
        state->debug_mode = 1;
        printf("Debug flag now off\n");
    }
}

void SetFileName(state *state)
{
    printf("Set file name: ");
    scanf("%s", state->file_name);
    if (state->debug_mode)
    {
        fprintf(stderr, "Debug => file name was set to: %s\n", state->file_name);
    }
}

void SetUnitSize(state *state)
{
    int size;
    printf("Set unit size: 1 , 2 or 4 ");
    scanf("%d", &size);
    if (size == 1 || size == 2 || size == 4)
    {
        state->unit_size = size;
        if (state->debug_mode)
        {
            fprintf(stderr, "Debug=>: unit size was set to: %d\n", size);
        }
    }
    else
    {
        fprintf(stderr, "Size choosen is not valid \n");
    }
}

void Quit(state *state)
{
    if (state->debug_mode)
    {
        fprintf(stderr, "QUITING \n");
    }
    exit(0);
}
void LoadIntoMemory(state *state)
{
    if (strcmp(state->file_name, "") == 0)
    {
        fprintf(stderr, "File name is empty \n");
        return;
    }
    FILE *file = fopen(state->file_name, "rb");
    if (file == NULL)
    {
        fprintf(stderr, "Error openning file : %s", state->file_name);
        return;
    }
    unsigned int location;
    size_t length;
    char input[128];
    printf("Please enter <location> <length> :");
    scanf("%x %zu", &location, &length);
    if (state->debug_mode)
    {
        fprintf(stderr, "FileName:%s Location: 0x%x  File length: %zu\n", state->file_name, location, length);
    }

    fseek(file, location, SEEK_SET);
    fread(state->mem_buf, state->unit_size, length, file);
    state->mem_count += length;
    fprintf(stdout, "LOADED %zu units into memory", length);
    fclose(file);
}

void ToggleDisplayMode(state *state)
{
    if (state->display_mode == 0)
    {
        state->display_mode = 1;
        fprintf(stderr, "Display flag now on, hexadecimal representation");
    }
    else
    {
        state->display_mode = 0;
        fprintf(stderr, "Display flag now off, decimal representation");
    }
}

void MemoryDisplay(state *state)
{
    unsigned int addr;
    size_t u;

    printf("Enter <address> <length> ");
    scanf("%x %zu", &addr, &u);
    if (state->display_mode)
    {
        print_units_decimal(stdout, (state->mem_buf + addr), u, state);
    }
    else
    {
        print_units_hexadecimal(stdout, (state->mem_buf + addr), u, state);
    }
}

void SaveIntoFile(state *state)
{
    if (strcmp(state->file_name, "") == 0)
    {
        fprintf(stderr, "No file name was loaded or empty");
        return;
    }
    FILE *file = fopen(state->file_name, "r+");
    if (file == NULL)
    {
        fprintf(stderr, "Couldn't open file\n");
        return;
    }
    printf("Enter <source_code> <target_location> <length>\n");
    unsigned int source_address;
    unsigned int target_location;
    unsigned int length;
    // char input[256];
    scanf("%x %x %d", &source_address, &target_location, &length);
    if (state->debug_mode){
        printf("Debug:  Source Address:%x Target Location:%x Length: %zu\n", source_address, target_location, length);
    }
    fseek(file , target_location ,SEEK_SET) ;
    fseek(file , 0 , SEEK_END) ;
    long fileSize = ftell(file) ;
    if(target_location > fileSize){
        printf("Error target location greater than fileSzie \n");
        fclose(file) ;
        return;
    }
    fseek(file , target_location , SEEK_SET) ;
    if(source_address == 0){
        fwrite(state->mem_buf , state->unit_size , length , file) ;
    }
    else
    {
        fwrite((void *)source_address , state->unit_size , length , file) ;
    }
    fclose(file) ;
 
}

void MemoryModify(state *state)
{
    int location;
    int val;
    char input[256];

    printf("Please enter <location> <val>");
    // fgets(input, sizeof(input), stdin);
    // sscanf(input, "%x", &location);

    // fgets(input, sizeof(input), stdin);
    scanf( "%x %x",&location, &val);

    if (state->debug_mode)
    {
        printf("Debug:  Location:%x Val: %x", location, val);
    }

    if (location > sizeof(state->mem_buf))
    {
        fprintf(stderr, "ERROR ! Target location is greater than file size\n");
        return;
    }
    else
    {
        *(unsigned int *)(state->mem_buf + location) = val;
        printf("Memory was modified in location %x with the Value:%x", location, val);
    }
}

int main()
{

    state s = {0, "", 1, {0}, 0};

    int option;
    int bound = sizeof(func_menu) / sizeof(func) - 1;
    while (1)
    {
        printf("Choose action:\n");
        for (size_t i = 0; i < bound; i++)
        {
            printf("%d. %s\n", i, func_menu[i].name);
        }
        printf(">");
        scanf("%d", &option);
        if (option > bound || option < 0)
        {
            fprintf(stderr, "Error invalid func number");
        }
        else
        {
            func_menu[option].function(&s); // activate the choosen func on s
        }
    }
    return 0;
}
