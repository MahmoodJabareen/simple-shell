#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>


int main( int argc,char** argv  ){
    int mypipe[2];
    pid_t child1 ;
    pid_t child2 ;
    if(pipe(mypipe) == -1)
        perror("ERROR CREATING A PIPE ");

    child1= fork() ;


    if(child1 == -1){
        perror("ERROR FORKING");
        _exit(EXIT_FAILURE);
    }
    if(child1 == 0){ // in child process
        close(STDOUT_FILENO) ;
        if(dup(mypipe[1]) == -1){
            perror("ERROR DUPLICATING ") ;
            _exit(EXIT_FAILURE) ;
        }
        if(close(mypipe[1]) == -1){
            perror("ERROR CLOSING FILE DESCRIPTOR") ;
            _exit(EXIT_FAILURE);
        }
        char* command[] = {"ls" ,"-l" ,NULL} ;
        if(execvp(command[0] , command) == -1){
            perror("ERROR EXECUTING : ls -l");
            _exit(EXIT_FAILURE) ;
        }
    }
    else{ // parent process
        if(close(mypipe[1]) == -1){
            perror("ERROR CLOSING WRITE END OF PIPE") ;
            _exit(EXIT_FAILURE);
        }
        child2 = fork();
        if(child2 == -1){
            perror("ERROR FORKING CHILD2") ;
            _exit(EXIT_FAILURE) ;
        }
        if(child2 == 0){
            close(STDIN_FILENO) ;
            if(dup(mypipe[0]) == -1){
            perror("ERROR DUPLICATING ") ;
            _exit(EXIT_FAILURE) ;
        }
        if(close(mypipe[0]) == -1){
            perror("ERROR CLOSING FILE DESCRIPTOR") ;
            _exit(EXIT_FAILURE);
        }
        char* command[] = {"tail" , "-n","2" ,NULL} ;
        if(execvp(command[0] , command) == -1){
            perror("ERROR EXECUTING : tail -n 2");
            _exit(EXIT_FAILURE) ;
        }
    }
    else{
        if(close(mypipe[0]) == -1){
            perror("ERROR CLOSING FILE DESCRIPTOR") ;
            _exit(EXIT_FAILURE);
        }
        waitpid(child1, NULL, 0);
        waitpid(child2, NULL, 0);
    }  
}
}
