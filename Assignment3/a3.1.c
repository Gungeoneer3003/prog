#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <unistd.h>
#include <sys/mman.h>

int main(int argc, char** argv) {
    //This program makes the following assumptions:
    // 1. The second program to be executed is compiled and in the same directory.
    // 2. The second program takes two arguments, the process number and the total number of processes.
    // 3. The number of processes is a small integer (less than 10).

    //Terminal commands
    // if(argc < 3) {
    //     printf("Insufficent arguments, please try again.\n");
    //     printf("Usage: ./a3.1 [secondProgram] count.\n");
    //     return 0;
    // }
    // char* prog2 = argv[1];
    // int count = atoi(argv[2]);
    
    //Hard coded inputs
    char* prog2 = "dummy";
    int count = 2;

    char countStr[2];
    char processStr[2];

    snprintf(countStr, sizeof(countStr), "%d", count);

    char *newArgv[3];
    newArgv[0] = processStr;
    newArgv[1] = countStr;
    newArgv[2] = NULL;
    

    int i = 0;
    while (i < count) {
        snprintf(processStr, sizeof(processStr), "%d", i);

        int pid = fork();

        if(pid == 0) {
            printf("Child starts\n");
            execv(prog2, newArgv);
            return 0;
        }
        i++;
    }

    printf("Parent waiting for child processes to complete.\n");
    while(wait(0) != -1);   
    

    printf("All child processes have completed.\n");
    
    //Conclude Program
    return 0;
}