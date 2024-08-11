#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <elf.h>

#define MAX_FILES 2

int debugMode = 0;

typedef struct
{
    int fileDescriptor;
    void *mappedFile;
    size_t fileSize;
    char fileName[256];
} ElfFile;

typedef struct
{
    char *name;
    void (*func)();
} function;

ElfFile elfFiles[MAX_FILES];
int examineELFCounter = 0;

void toggleDebugMode();
void examineELFFile();
void printSectionNames();
void printSymbols();
void checkFilesForMerge();
void mergeELFFiles();
void quit();

void toggleDebugMode()
{
    debugMode = !debugMode;
    printf("Debug mode is now %s\n", debugMode ? "on" : "off");
}

void quit()
{
    for (int i = 0; i < MAX_FILES; i++)
    {
        if (elfFiles[i].mappedFile != NULL)
        {
            munmap(elfFiles[i].mappedFile, elfFiles[i].fileSize);
            close(elfFiles[i].fileDescriptor);
        }
    }
    printf("QUITING\n");
    exit(0);
}

void examineELFFile()
{
    if (examineELFCounter >= MAX_FILES)
    {
        printf("Cannot examine more then two elf fiiles\n");
        return;
    }
    char fileName[256];
    printf("Endter ELF file name: ");
    scanf("%s", fileName);

    int fileDescriptor = open(fileName, O_RDONLY);
    if (fileDescriptor == -1)
    {
        perror("error opening");
        return;
    }
    struct stat st;
    if (fstat(fileDescriptor, &st) == -1)
    {
        perror("fstat");
        close(fileDescriptor);
        return;
    }
    void *map_start = mmap(NULL, st.st_size, PROT_READ, MAP_PRIVATE, fileDescriptor, 0);
    if (map_start == MAP_FAILED)
    {
        perror("Error maping file");
        close(fileDescriptor);
        return;
    }

    Elf32_Ehdr *header = (Elf32_Ehdr *)map_start;

    if (header->e_ident[EI_MAG0] != ELFMAG0 ||
        header->e_ident[EI_MAG1] != ELFMAG1 ||
        header->e_ident[EI_MAG2] != ELFMAG2 ||
        header->e_ident[EI_MAG3] != ELFMAG3)
    {
        fprintf(stderr, "Not an ELF file\n");
        munmap(map_start, st.st_size);
        close(fileDescriptor);
        return;
    }
    printf("Bytes 1, 2, 3 of the magic number (in ASCII): %c%c%c\n",
           header->e_ident[EI_MAG1], header->e_ident[EI_MAG2], header->e_ident[EI_MAG3]);
    printf("Data encoding scheme of the object file: %s\n",
           (header->e_ident[EI_DATA] == ELFDATA2LSB) ? "Little Endian" : "Big Endian");
    printf("Section header offset: %d\n", header->e_shoff);
    printf("Number of section headers: %d\n", header->e_shnum);
    printf("Size of section header: %d\n", header->e_shentsize);
    printf("Program header offset: %d\n", header->e_phoff);
    printf("Number of program headers: %d\n", header->e_phnum);
    printf("Size of program header: %d\n", header->e_phentsize);

    elfFiles[examineELFCounter].fileDescriptor = fileDescriptor;
    elfFiles[examineELFCounter].mappedFile = map_start;
    elfFiles[examineELFCounter].fileSize = st.st_size;
    strncpy(elfFiles[examineELFCounter].fileName, fileName, sizeof(elfFiles[examineELFCounter].fileName) - 1);

    examineELFCounter++;
}
void printSectionNames()
{
    if (elfFiles[0].mappedFile == NULL && elfFiles[1].mappedFile == NULL)
    {
        printf("No files was loaded \n ");
        return;
    }

    for (int i = 0; i < examineELFCounter; i++)
    {
        if (elfFiles[i].mappedFile == NULL)
        {
            printf("No ELF file loaded.\n");
            return;
        }

        Elf32_Ehdr *elf_header = (Elf32_Ehdr *)elfFiles[i].mappedFile;
        Elf32_Shdr *section_header_table = (Elf32_Shdr *)((uintptr_t)elfFiles[i].mappedFile + elf_header->e_shoff);
        char *string_table = (char *)((uintptr_t)elfFiles[i].mappedFile + section_header_table[elf_header->e_shstrndx].sh_offset);

        printf("File %s\n", elfFiles[i].fileName);
        for (int j = 0; j < elf_header->e_shnum; j++)
        {
            printf("[%d] %s 0x%x 0x%x 0x%x %d\n",
                   j,
                   &string_table[section_header_table[j].sh_name],
                   section_header_table[j].sh_addr,
                   section_header_table[j].sh_offset,
                   section_header_table[j].sh_size,
                   section_header_table[j].sh_type);
        }
    }
}
void printSymbols()
{
    if (elfFiles[0].mappedFile == NULL && elfFiles[1].mappedFile == NULL)
    {
        printf("No files were loaded.\n");
        return;
    }

    for (int i = 0; i < examineELFCounter; i++)
    {
        if (elfFiles[i].mappedFile == NULL)
        {
            printf("No ELF file loaded.\n");
            return;
        }

        Elf32_Ehdr *elf_header = (Elf32_Ehdr *)elfFiles[i].mappedFile;
        Elf32_Shdr *section_header_table = (Elf32_Shdr *)((uintptr_t)elfFiles[i].mappedFile + elf_header->e_shoff);
        char *string_table = (char *)((uintptr_t)elfFiles[i].mappedFile + section_header_table[elf_header->e_shstrndx].sh_offset);

        for (size_t j = 0; j < elf_header->e_shnum; j++)
        {
            if (section_header_table[j].sh_type == SHT_SYMTAB)
            {
                Elf32_Sym *symbolTable = (Elf32_Sym *)((uintptr_t)elfFiles[i].mappedFile + section_header_table[j].sh_offset);
                int numOfSymbols = section_header_table[j].sh_size / sizeof(Elf32_Sym);

                printf("File %s\n", elfFiles[i].fileName);

                for (size_t k = 0; k < numOfSymbols; k++)
                {

                    const char *section_name = (symbolTable[k].st_shndx < elf_header->e_shnum)
                                                   ? &string_table[section_header_table[symbolTable[k].st_shndx].sh_name]
                                                   : "UND";

                    printf("[%zu] 0x%x %d %s %s\n",
                           k,
                           symbolTable[k].st_value,
                           symbolTable[k].st_shndx,
                           section_name,
                           &string_table[symbolTable[k].st_name]);
                }

                if (debugMode)
                {
                    printf("Symbol table size: %d\n", section_header_table[j].sh_size);
                    printf("Number of symbols: %d\n", numOfSymbols);
                }
            }
        }
    }
}
void checkFilesForMerge()
{
    if (elfFiles[0].mappedFile == NULL || elfFiles[1].mappedFile == NULL)
    {
        printf("Files were not opened.\n");
        return;
    }

    Elf32_Ehdr *elf_header1 = (Elf32_Ehdr *)elfFiles[0].mappedFile;
    Elf32_Ehdr *elf_header2 = (Elf32_Ehdr *)elfFiles[1].mappedFile;
    Elf32_Shdr *section_header_table1 = (Elf32_Shdr *)((uintptr_t)elfFiles[0].mappedFile + elf_header1->e_shoff);
    Elf32_Shdr *section_header_table2 = (Elf32_Shdr *)((uintptr_t)elfFiles[1].mappedFile + elf_header2->e_shoff);

    int symbTabIndex1 = -1;
    int symbTabIndex2 = -1;

    for (int i = 0; i < elf_header1->e_shnum; i++)
    {
        if (section_header_table1[i].sh_type == SHT_SYMTAB)
        {
            if (symbTabIndex1 != -1)
            {
                printf("Feature not supported: file 1 has more than one symbol table.\n");
                return;
            }
            symbTabIndex1 = i;
        }
    }

    for (int i = 0; i < elf_header2->e_shnum; i++)
    {
        if (section_header_table2[i].sh_type == SHT_SYMTAB)
        {
            if (symbTabIndex2 != -1)
            {
                printf("Feature not supported: file 2 has more than one symbol table.\n");
                return;
            }
            symbTabIndex2 = i;
        }
    }

    if (symbTabIndex1 == -1 || symbTabIndex2 == -1)
    {
        printf("Feature not supported: one or both files lack a symbol table.\n");
        return;
    }

    Elf32_Sym *symbolTable1 = (Elf32_Sym *)((uintptr_t)elfFiles[0].mappedFile + section_header_table1[symbTabIndex1].sh_offset);
    int numOfSymbols1 = section_header_table1[symbTabIndex1].sh_size / sizeof(Elf32_Sym);
    Elf32_Sym *symbolTable2 = (Elf32_Sym *)((uintptr_t)elfFiles[1].mappedFile + section_header_table2[symbTabIndex2].sh_offset);
    int numOfSymbols2 = section_header_table2[symbTabIndex2].sh_size / sizeof(Elf32_Sym);

    char *strTab1 = (char *)((uintptr_t)elfFiles[0].mappedFile + section_header_table1[section_header_table1[symbTabIndex1].sh_link].sh_offset);
    char *strTab2 = (char *)((uintptr_t)elfFiles[1].mappedFile + section_header_table2[section_header_table2[symbTabIndex2].sh_link].sh_offset);

    for (int i = 1; i < numOfSymbols1; i++)
    {
        Elf32_Sym *symbol1 = &symbolTable1[i];
        const char *symName1 = &strTab1[symbol1->st_name];
        if (strcmp(symName1, "") == 0)
        {
            continue;
        }

        if (symbol1->st_shndx == SHN_UNDEF)
        {
            int found = 0;
            int defined = -1;
            for (int j = 1; j < numOfSymbols2; j++)
            {
                Elf32_Sym *symbol2 = &symbolTable2[j];
                const char *symName2 = &strTab2[symbol2->st_name];
                if (strcmp(symName1, symName2) == 0)
                {
                    found = 1;
                    defined = symbol2->st_shndx;
                    break;
                }
            }

            if (found == 0 || defined == SHN_UNDEF)
            {
                printf("Symbol %s undefined in both files.\nCan't Merge\n", symName1);
                return;
            }
        }
        else
        {
            int found = 0;
            int defined = -1;
            for (int j = 1; j < numOfSymbols2; j++)
            {
                Elf32_Sym *symbol2 = &symbolTable2[j];
                const char *symName2 = &strTab2[symbol2->st_name];
                if (strcmp(symName1, symName2) == 0)
                {
                    found = 1;
                    defined = symbol2->st_shndx;
                    break;
                }
            }

            if (found == 1 && defined != SHN_UNDEF)
            {
                printf("Symbol %s multiply defined.\nCan't Merge\n", symName1);
                return;
            }
        }
    }

    for (int i = 1; i < numOfSymbols2; i++)
    {
        Elf32_Sym *symbol2 = &symbolTable2[i];
        const char *symName2 = &strTab2[symbol2->st_name];
        if (strcmp(symName2, "") == 0)
        {
            continue;
        }

        if (symbol2->st_shndx == SHN_UNDEF)
        {
            int found = 0;
            int defined = -1;
            for (int j = 1; j < numOfSymbols1; j++)
            {
                Elf32_Sym *symbol1 = &symbolTable1[j];
                const char *symName1 = &strTab1[symbol1->st_name];
                if (strcmp(symName2, symName1) == 0)
                {
                    found = 1;
                    defined = symbol1->st_shndx;
                    break;
                }
            }

            if (found == 0 || defined == SHN_UNDEF)
            {
                printf("Symbol %s undefined in both files.\nCan't Merge\n", symName2);
                return;
            }
        }
        else
        {
            int found = 0;
            int defined = -1;
            for (int j = 1; j < numOfSymbols1; j++)
            {
                Elf32_Sym *symbol1 = &symbolTable1[j];
                const char *symName1 = &strTab1[symbol1->st_name];
                if (strcmp(symName2, symName1) == 0)
                {
                    found = 1;
                    defined = symbol1->st_shndx;
                    break;
                }
            }

            if (found == 1 && defined != SHN_UNDEF)
            {
                printf("Symbol %s multiply defined.\nCan't Merge\n", symName2);
                return;
            }
        }
    }
}
void mergeELFFiles() {}

int main()
{
    function menu[] = {
        {"Toggle Debug Mode", toggleDebugMode},
        {"Examine ELF File", examineELFFile},
        {"Print Section Names", printSectionNames},
        {"Print Symbols", printSymbols},
        {"Check Files for Merge", checkFilesForMerge},
        {"Merge ELF Files", mergeELFFiles},
        {"Quit", quit},
        {NULL, NULL}};

    for (int i = 0; i < MAX_FILES; i++)
    {
        elfFiles[i].fileDescriptor = -1;
        elfFiles[i].mappedFile = NULL;
        elfFiles[i].fileSize = 0;
        memset(elfFiles[i].fileName, 0, sizeof(elfFiles[i].fileName));
    }

    int choice;
    while (1)
    {
        printf("Choose action:\n");
        for (int i = 0; menu[i].name != NULL; i++)
        {
            printf("%d-%s\n", i, menu[i].name);
        }
        scanf("%d", &choice);
        if (choice >= 0 && choice <= 6)
        {
            menu[choice].func();
        }
        else
        {
            printf("Invalid choice\n");
        }
    }

    return 0;
}