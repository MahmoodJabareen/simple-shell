#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

bool is_letter(char c) {
    return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z');
}

bool is_digit(char c) {
    return c >= '0' && c <= '9';
}

int main(int argc , char *argv[]){
    char *infile = NULL;
    bool inputfile = false;
    char *outfile = NULL;
    bool outputfile = false;
    char *encoding_value = NULL;
    char *encoding_key = NULL;
    int debug_mode = 1;
    char *input_value = NULL;

    //Command line
    for(int i=1 ; i< argc ; i++){
        if(strcmp(argv[i] , "-I") == 0 && i+1<argc ){ // Here is input file name
            infile = argv[i+1];
            inputfile = true;
            i++;
        }else if(strcmp(argv[i] , "-O") == 0 && i+1<argc ){ // Here is output file name
            outfile = argv[i+1];
            outputfile = true;
            i++;
        } else if (strncmp(argv[i], "+e", 2) == 0 || strncmp(argv[i], "-e", 2) == 0) {
            encoding_value = argv[i] + 2;
            encoding_key = argv[i];
        } else if (strcmp(argv[i], "-D") == 0) {
            debug_mode = 0; // Turn off debug mode
        } else if (strcmp(argv[i], "+D") == 0) {
            debug_mode = 1; // Turn on debug mode
        } else {
            input_value = argv[i];
        }
    }
    if(debug_mode){ //The Debug-mode
        fprintf(stderr , "Command line arguments : \n");
        for(int i =0 ; i< argc ; i++){
            fprintf(stderr , "%s\n" , argv[i]);
        }
    }
    FILE *in = stdin;
    if(infile != NULL){
        in = fopen(infile , "r");
        if(in == NULL){
            fprintf(stderr , "Error opening the input file \n");
            return 1;
        }
    }
    FILE *out = stdout;
    if (outfile != NULL) {
        out = fopen(outfile, "w");
        if (out == NULL) {
            fprintf(stderr, "Error opening output file.\n");
            return 1;
        }
    }
    // With encoding key
    if(encoding_key != NULL){
        //int key_length = strlen(encoding_value);
        int key_length = 0;
        while (encoding_value[key_length] != '\0') {
            key_length++;
        }
        int key_index = 0;
        int c = 0;
        if(inputfile){ //Case 1
            while((c = fgetc(in)) != EOF) {
                if (is_letter(c)) { //Letter 
                    int key_digit = encoding_value[key_index] - '0';
                    if (strncmp(encoding_key, "+e", 2) == 0) { //Addition
                        c = (c - 'a' + key_digit) % 26 + 'a'; 
                    } else if (strncmp(encoding_key, "-e", 2) == 0) { //Subtraction
                        c = (c - 'a' - key_digit + 26) % 26 + 'a';  
                    }
                    key_index = (key_index + 1) % key_length; //Move to the next key digit
                } else if (is_digit(c)) { //Number
                    int key_digit = encoding_value[key_index] - '0';
                    if (strncmp(encoding_key, "+e", 2) == 0) { //Addition
                        c = (c - '0' + key_digit) % 10 + '0';  
                    } else if (strncmp(encoding_key, "-e", 2) == 0) { //Subtraction 
                        c = (c - '0' - key_digit + 10) % 10 + '0';
                    }
                    key_index = (key_index + 1) % key_length; //Move to the next key digit
                }
                if(outputfile){fputc (c,out);} //Case 1.1
                else{putchar (c);} //Case 1.2
                }
            }else{ //Case 2
                while ((c = getchar()) != EOF) {
                if (is_letter(c)) { //Letter
                    int key_digit = encoding_value[key_index] - '0';
                    if (strncmp(encoding_key, "+e", 2) == 0) { //Addition
                        c = (c - 'a' + key_digit) % 26 + 'a';
                    } else if (strncmp(encoding_key, "-e", 2) == 0) { //Subtraction
                        c = (c - 'a' - key_digit + 26) % 26 + 'a';  
                    }
                    key_index = (key_index + 1) % key_length; //Move to the next key digit
                } else if (is_digit(c)) { //Number
                    int key_digit = encoding_value[key_index] - '0';
                    if (strncmp(encoding_key, "+e", 2) == 0) { //Addition
                        c = (c - '0' + key_digit) % 10 + '0';
                    } else if (strncmp(encoding_key, "-e", 2) == 0) { //Subtraction
                        c = (c - '0' - key_digit + 10) % 10 + '0';
                    }
                    key_index = (key_index + 1) % key_length; //Move to the next key digit
                }
                if (outputfile) {fputc(c, out);} //Case 2.1
                else { putchar(c);} //Case 2.2
            }
        }
    } else { //Without encoding key
       
        int c;
        if (inputfile) {
            while ((c = fgetc(in)) != EOF) {
                if (outputfile) { fputc(c, out);} else { putchar(c);}
            }
        } else if (input_value != NULL) {
            // Print the input value if specified
            if (outputfile) { fputs(input_value, out);} else {printf("%s", input_value);}
        } else {
            // No input file or value provided
            fprintf(stderr, "No input file or value provided.\n");
        }
    }
    // Close files
    if (in != stdin) {fclose(in);}
    if (out != stdout) {fclose(out);}
    return 0;
}