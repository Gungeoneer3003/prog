#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <sys/mman.h>
#include <sys/wait.h>
#include <sys/types.h>

int save_stdin;
int fd[2];
void signalHandler(int i) {
    dup2(fd[0], STDIN_FILENO);
    return;
}

void signalhandler2(int i) {
    dup2(save_stdin, STDIN_FILENO);
}

int main() {
    int* qSig = (int*)mmap(NULL,4,PROT_READ|PROT_WRITE,MAP_SHARED|MAP_ANONYMOUS,-1,0);
    *qSig = 0;
    int* wSig = (int*)mmap(NULL,4,PROT_READ|PROT_WRITE,MAP_SHARED|MAP_ANONYMOUS,-1,0);
    *wSig = 0;

    pipe(fd);
    int parentPID = getpid();
    char text[100];

    

    int pid = fork();
    if (pid == 0) {
        while(*qSig == 0) {
            sleep(5);
            
            if(*wSig != 0) {
                *wSig = 0;
                continue;
            }

            //Take control of stdin
            kill(parentPID, SIGUSR1);
            close(fd[0]);
            write(fd[1], "Inactivity Detected!", 21);
            close(fd[1]);
            sleep(1);
            kill(parentPID, SIGUSR2);
        }

        return 0;
    }

    signal(SIGUSR1, signalHandler);
    signal(SIGUSR2, signalhandler2);

    //parent
    save_stdin = dup(STDIN_FILENO);
    while(1) {
        
        //Start reading in text
        size_t x = read(STDIN_FILENO, text, 100);
        text[x-1] = 0;
        //Check if the text is quit, break if so
        *wSig += 1;
        if(strcmp(text, "quit") == 0) {
            *qSig += 1;
            break;
        }

        //Print resulting text with the !
        if(strcmp("Inactivity Detected!", text) != 0)
            printf("!%s!\n", text);
        else
            printf("%s\n", text);

        //signalhandler2(0);
        //dup2(save_stdin, STDIN_FILENO);    
    }

    //wait for the child to die
    wait(0);

    munmap(wSig, 4);
    munmap(qSig, 4);
    return 0;
}