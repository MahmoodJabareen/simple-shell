#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#define MAX_INPUT 100
#define BUFFER_SIZE 10240


typedef struct virus {
    unsigned short SigSize;
    char virusName[16];
    unsigned char* sig;
} virus;

typedef struct link {
    struct link *nextVirus;
    virus *vir;
} link;

void printHexa(unsigned char *buffer, int length, FILE *output) {
    for (int i = 0; i < length; i++) {
        fprintf(output, "%02X ", buffer[i]);
    }
    fprintf(output, "\n");
}

void SetSigFileName(char* fileName){
    printf("Enter File Name : ") ;
    if(fgets(fileName ,256,stdin) != NULL){
        size_t len = strlen(fileName) ;
        if(len > 0 && fileName[len -1] =='\n')
            fileName[len-1] = '\0' ;
    }
}
virus* readVirus(FILE* file){
    virus* v = (virus*)malloc(sizeof(virus));
    if (fread(&v->SigSize, 2, 1, file) != 1) {
        free(v);
        return NULL;
    }
    if (fread(v->virusName, 1, 16, file) != 16) {
        free(v);
        return NULL;
    }
    v->sig = (unsigned char*)malloc(v->SigSize); 
    if (fread(v->sig, 1, v->SigSize, file) != v->SigSize) {
        free(v->sig);
        free(v);
        return NULL;
    }
    return v;
}

void printVirus(virus* v, FILE* file){
    fprintf(file, "Virus Name: %s\n", v->virusName);
    fprintf(file, "Virus Sig size: %d\n", v->SigSize);
    fprintf(file, "Virus Signature: ");
    for (size_t i = 0; i < v->SigSize; i++) {
        fprintf(file, "%02X ", v->sig[i]);
    }
    fprintf(file, "\n");
}
void list_print(link *virus_list, FILE* file) {
    link *current = virus_list;
    while (current != NULL) {
        printVirus(current->vir, file);
        current = current->nextVirus;
    }
}
link* list_append(link* virus_list , virus* data){
    link* newLink = (link*)malloc(sizeof(link)) ;
    if (newLink == NULL) {
        perror("Memory allocation failed");
        return virus_list;
    }
    newLink->vir = data ;
    newLink->nextVirus = virus_list ;
    return newLink ;
}
void virus_free(virus* vir){
    free(vir->sig) ;
    free(vir) ;
}
void list_free(link *virus_list){
    if(virus_list != NULL){ 
        virus_free(virus_list->vir) ;
        list_free(virus_list->nextVirus) ;
        free(virus_list) ;
    }
}
void LoadSignatures(link** virus_list, char* fileName){
    FILE* file = fopen(fileName , "rb") ;

    if (!file) {
        perror("Error opening file");
        fclose(file);
        return;
    }
    char magic[4];
    fread(magic, 1, 4, file);
    if (strncmp(magic, "VIRL", 4) != 0 && strncmp(magic, "VIRB", 4) != 0) {
        printf("Error: incorrect magic number\n");
        fclose(file);
        return;
    }
    virus* vir;
    while ((vir = readVirus(file)) != NULL) {
        *virus_list = list_append(*virus_list, vir);
    }
    fclose(file);

}
void detect_virus(char *buffer, unsigned int size, link *virus_list){
    link* cur= virus_list;
    while(cur !=NULL){
        virus* v = cur->vir;
        for(int i = 0 ; i<=size - v->SigSize ; i++){
            if (memcmp(buffer + i, v->sig, v->SigSize) == 0) {
                printf("Virus detected!\n");
                printf("Starting byte location: %u\n", i);
                printf("Virus name: %s\n", v->virusName);
                printf("Virus signature size: %u\n", v->SigSize);
            }
        }
        cur=cur->nextVirus;
    }
}
void neutralize_virus(char *fileName, int signatureOffset){
    FILE* file = fopen(fileName,"r+b");
    if(!file){
        perror("error openning file");
    }
    if(fseek(file,signatureOffset,SEEK_SET)!=0){
        perror("error seeking");
        fclose(file);
        return;
    }
    char ret= 0xC3;
    if(fwrite(&ret,sizeof(char) , 1,file) !=1){
        perror("error editing") ;
    }
    fclose(file); 

}
void Quit(link **virus_list) {
    list_free(*virus_list);
    exit(0);
}
void setSigFileNameWrapper(link **virus_list ,char* suspectedFileName) {
    char fileName[256];
    SetSigFileName(fileName);
    printf("Signatures file name set to: %s\n", fileName);
}

void loadSignaturesWrapper(link **virus_list ,char* suspectedFileName) {
    char fileName[256] = "signatures-L";
    LoadSignatures(virus_list, fileName);
}
void printSignaturesWrapper(link **virus_list,char* suspectedFileName) {
    list_print(*virus_list, stdout);
}

void detectVirusWrapper(link **virus_list,char* suspectedFileName) {
    if(virus_list == NULL){
        perror("No viruses were loaded");
        return;
    }
    FILE* file = fopen(suspectedFileName , "rb");
    if(!file){
        perror("Error oppeneing suspected file") ;
        return;
    }
    char buffer[BUFFER_SIZE];
    int bytes = fread(buffer , 1 ,BUFFER_SIZE,file);
    if(bytes >0){
        detect_virus((char *)buffer,bytes ,*virus_list);
    }
    fclose(file);
}

void fixFile(link **virus_list, char *suspectedFileName) {
    FILE *file = fopen(suspectedFileName, "r+b");
    if (!file) {
        perror("Error opening file");
        return;
    }
    unsigned char buffer[BUFFER_SIZE];
    size_t size = fread(buffer, 1, BUFFER_SIZE, file);
    fclose(file);
      if (size == 0) {
        perror("Error reading file");
        return;
    }
    link *current = *virus_list;
    while (current != NULL) {
        virus *v = current->vir;
        for (size_t i = 0; i <= size - v->SigSize; i++) {
            if (memcmp(buffer + i, v->sig, v->SigSize) == 0) {
                     printf("Neutralizing virus...\n");
                printf("Starting byte location: %u\n", i);
                printf("Virus name: %s\n", v->virusName);
                printf("Virus signature size: %u\n\n", v->SigSize);

                // Neutralize the virus
                neutralize_virus(suspectedFileName, i);
            }
        }
        current = current->nextVirus;
    }
}

void quitWrapper(link **virus_list,char* suspectedFileName) {
    Quit(virus_list);
}

typedef struct fun_desc {
    char *name;
    void (*fun)(link **virus_list ,char* suspectedFileName);
} fun_desc;

int main(int argc , char** argv){
    link *virus_list = NULL;
    char* suspectedFileName = argv[1] ; 
    fun_desc menu[] = {
        {"Set signatures file name", setSigFileNameWrapper},
        {"Load Signatures", loadSignaturesWrapper},
        {"Print Signatures", printSignaturesWrapper},
        {"Detect Virus", detectVirusWrapper},
        {"Fix File", fixFile},
        {"Quit", quitWrapper},
        {NULL, NULL}
    };
    char input[MAX_INPUT];
    int bound = (sizeof(menu) / sizeof(struct fun_desc)) -1 ;
    
    while (1) {
        printf("Select operation from the following menu:\n");
        for (int i = 0; i < bound; i++) {
            printf("%d: %s\n", i, menu[i].name);
        }
        printf("Option: ");
        if (fgets(input, MAX_INPUT, stdin) == NULL)
            break;
        
        int funcNum;
        if (sscanf(input, "%d", &funcNum) != 1 || funcNum < 0 || funcNum > bound) {
            printf("Not within bounds\n");
            continue;
        }
      printf("Within bounds\n");
        menu[funcNum].fun(&virus_list,suspectedFileName);
    }
    
    return 0;
}