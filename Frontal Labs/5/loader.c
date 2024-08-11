#include <stddef.h>
#include <stdio.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <stdlib.h>
#include <elf.h>
#include <unistd.h>
#include <string.h>

extern int startup(int argc, char** argv, int (*func)(int, char**));
//task 0
void printer_phdr(Elf32_Phdr *phdr, int index) {
    printf("Program header number %d at address 0x%x\n", index, (unsigned int)phdr);
}

int foreach_phdr(void *map_start, void (*func)(Elf32_Phdr *, int), int arg) {
    Elf32_Ehdr *headers = (Elf32_Ehdr *)map_start;
    Elf32_Phdr *p_header_group = (Elf32_Phdr *)(map_start + headers->e_phoff);
    int len =headers->e_phnum;
    for (int i = 0; i < len; i++) {
        func(&p_header_group[i], arg);
    }
    return 0;
}

void print_appropriate_protection_flags(Elf32_Phdr *phdr){
    if (phdr->p_type != PT_LOAD) {
        printf("header type  %d not exsist in mapping\n", phdr->p_type);
        return;
    }
    char flags[4] = {' ', ' ', ' ', '\0'};
char* flagNames = "RWE";

for (int i = 0; i < 3; i++) {
    if (phdr->p_flags & (1 << i))
        flags[i] = flagNames[i];
}


    int prot_flags = 0;
    if (phdr->p_flags & PF_R)
        prot_flags |= PROT_READ;
    if (phdr->p_flags & PF_W)
        prot_flags |= PROT_WRITE;
    if (phdr->p_flags & PF_X)
        prot_flags |= PROT_EXEC;

    int mapping_flags = MAP_PRIVATE | MAP_FIXED;
    if (phdr->p_flags & PF_W)
        mapping_flags |= MAP_SHARED;

}
void print_flags(Elf32_Phdr *phdr){
    printf("%c%c%c ", 
           (phdr->p_flags & PF_R) ? 'R' : ' ', 
           (phdr->p_flags & PF_W) ? 'W' : ' ', 
           (phdr->p_flags & PF_X) ? 'E' : ' ');
}
void print_type(Elf32_Phdr *phdr){
    printf("%s ", 
            (phdr->p_type == PT_DYNAMIC) ? "DYNAMIC" :
            (phdr->p_type == PT_NULL) ? "NULL" :
           (phdr->p_type == PT_LOAD) ? "LOAD" :
           (phdr->p_type == PT_NOTE) ? "NOTE" :
           (phdr->p_type == PT_GNU_STACK) ? "GNU_STACK" :
           (phdr->p_type == PT_GNU_RELRO) ? "GNU_RELRO" :
           (phdr->p_type == PT_PHDR) ? "PHDR" :
           (phdr->p_type == PT_INTERP) ? "INTERP" : "UNDECLARED");
}
void print_offset(Elf32_Phdr *phdr){
printf("0x%06x ",phdr->p_offset);
}
void print_addresses(Elf32_Phdr *phdr){
    printf("0x%08x 0x%08x ", phdr->p_vaddr, phdr->p_paddr);
}
void sizes_print(Elf32_Phdr *phdr){
    printf("0x%05x 0x%05x ",phdr->p_filesz, phdr->p_memsz);
}
void print_alignment(Elf32_Phdr *phdr){
    printf("0x%x\n", phdr->p_align);
}
void display_phdr_info(Elf32_Phdr *phdr, int index) {
    print_type(phdr);
    print_offset(phdr);
    print_addresses(phdr);
    sizes_print(phdr);
    print_flags(phdr);
    print_alignment(phdr);
    print_appropriate_protection_flags(phdr);
}

void set_protection_flags(Elf32_Phdr *phdr, int *prot) {
    *prot = 0;  // Initialize protection flags

    switch (phdr->p_flags) {
        case PF_R:
            *prot |= PROT_READ;
            break;
        case PF_W:
            *prot |= PROT_WRITE;
            break;
        case PF_X:
            *prot |= PROT_EXEC;
            break;
        default:
            if (phdr->p_flags & PF_R) *prot |= PROT_READ;
            if (phdr->p_flags & PF_W) *prot |= PROT_WRITE;
            if (phdr->p_flags & PF_X) *prot |= PROT_EXEC;
            break;
    }
}
void load_phdr(Elf32_Phdr *phdr, int fd) {
    off_t offset = phdr->p_offset & 0xfffff000;
    size_t padding = phdr->p_vaddr & 0xfff;

    void* vaddr = (void*)(phdr->p_vaddr & 0xfffff000);
    size_t filesz = phdr->p_filesz;
    size_t memsz = phdr->p_memsz;
    int prot = 0;

    set_protection_flags(phdr, &prot);
    void *map = mmap(vaddr, memsz + padding, prot, MAP_PRIVATE | MAP_FIXED, fd, offset);
    if (map == MAP_FAILED) {
        perror("error mmap");
        exit(1);
    }
    display_phdr_info(phdr, fd);
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <elf-file>\n", argv[0]);
        return EXIT_FAILURE;
    }

    char *filename = argv[1];
    int fd = open(filename, O_RDONLY);
    if (fd == -1) {
        perror("Error opening file");
        return EXIT_FAILURE;
    }

    struct stat st;
    if (fstat(fd, &st) == -1) {
        perror("Error getting file size");
        close(fd);
        return EXIT_FAILURE;
    }

    void *map_start = mmap(NULL, st.st_size, PROT_READ, MAP_PRIVATE, fd, 0);
    if (map_start == MAP_FAILED) {
        perror("Error mapping file");
        close(fd);
        return EXIT_FAILURE;
    }

    Elf32_Ehdr *ehdr = (Elf32_Ehdr *)map_start;
    Elf32_Phdr *phdr = (Elf32_Phdr *)(map_start + ehdr->e_phoff);
    printf("Type    Offset   VirtAddr   PhysAddr   FileSiz MemSiz Flg Align\n");
    int result = foreach_phdr(map_start, load_phdr, fd);
    close(fd);    
    startup(argc - 1, argv + 1, (void *)(ehdr->e_entry));
    munmap(map_start, st.st_size);
    return EXIT_SUCCESS;
}
