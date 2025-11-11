#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <unistd.h>
#include <sys/mman.h>

int main(int argc, char** argv) {
    //Terminal commands
    // if(argc < 3) {
    //     printf("Insufficent arguments, please try again.\n");
    //     printf("Usage: ./a3.1 [secondProgram] count.\n");
    //     return 0;
    // }
    // char* prog2 = argv[1];
    // int count = atoi(argv[2]);
    
    //Hard coded inputs
    char* prog2 = "dummy.c";
    int count = 2;
    
    //Error handling for checking if the program exists

    
    int i = 0;
    while (i < count) {
        int pid = fork();

        if(pid == 0) {
            printf("Child starts\n");
            execv(prog2);
            printf("Child Ends\n");
            return 0;
        }
        i++;
    }

    printf("Parent Starts\n");
    printf("Skibbidy Ohio Rizz.\n");
    printf("Parent Ends Here\n");
    while(wait(0) != -1);   
    

    
    //Conclude Program
    return 0;
}