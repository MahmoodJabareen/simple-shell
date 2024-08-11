#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>
#include <elf.h>

int foreach_phdr(void *map_start, void (*func)(Elf32_Phdr *, int), int arg) {
    Elf32_Ehdr *elf_header = (Elf32_Ehdr *)map_start;
    
    // Verify the ELF magic number
    if (elf_header->e_ident[EI_MAG0] != ELFMAG0 || 
        elf_header->e_ident[EI_MAG1] != ELFMAG1 || 
        elf_header->e_ident[EI_MAG2] != ELFMAG2 || 
        elf_header->e_ident[EI_MAG3] != ELFMAG3) {
        fprintf(stderr, "Error: Not a valid ELF file\n");
        return -1;
    }

    Elf32_Phdr *program_headers = (Elf32_Phdr *)(map_start + elf_header->e_phoff);
    for (int i = 0; i < elf_header->e_phnum; i++) {
        func(&program_headers[i], i);
    }
    
    return 0;
}
void print_phdr(Elf32_Phdr *phdr, int index) {
    printf("Program header number %d at address %p\n", index, (void *)phdr);
}
int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <ELF file>\n", argv[0]);
        return 1;
    }

    const char *filename = argv[1];
    int fd = open(filename, O_RDONLY);
    if (fd < 0) {
        perror("open");
        return 1;
    }

    struct stat st;
    if (fstat(fd, &st) < 0) {
        perror("fstat");
        close(fd);
        return 1;
    }

    void *map_start = mmap(NULL, st.st_size, PROT_READ, MAP_PRIVATE, fd, 0);
    if (map_start == MAP_FAILED) {
        perror("mmap");
        close(fd);
        return 1;
    }

    if (foreach_phdr(map_start, print_phdr, 0) < 0) {
        fprintf(stderr, "Error processing ELF file\n");
    }

    munmap(map_start, st.st_size);
    close(fd);
    return 0;
}
