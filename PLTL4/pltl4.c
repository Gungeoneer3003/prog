#include <unistd.h>
#include <signal.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/wait.h>
#include <sys/types.h>

int fd[2];
void signalhandler(int i) {
    dup2(fd[0], STDIN_FILENO);
    printf("Overriding stdin here\n");
}



int main(int argc, char *argv[]) {
    if(argc != 3) {
        printf("Bad argument count");
        return 0;
    }

    char* option1 = argv[1];
    char* option2 = argv[2];
    
    pipe(fd);
    int parentPID = getpid();
    char text[100];

    
    if(fork() == 0) {
        FILE *fp = fopen(option1, "rb");
        if(fp == 0) {
            printf("Bad input file");
            return 0;
        }

        char *data = (char*)malloc(100);

        size_t s = fread(data, 1, 100, fp);
        fclose(fp);

        write(fd[1], data, s);
        close(fd[1]);
        free(data);
        return 0;
    }
    else {

        char* newData = (char*)malloc(100);
        size_t x = read(fd[0], newData, 100);
        newData[x] = '\0';

        if(strcmp(option2, "terminal") == 0) {
            write(STDOUT_FILENO, newData, 100);
            
        }
        else {
            FILE *out = fopen(option2, "wb");
            fwrite(newData, 1, x, out);
            fclose(out);
        }

        free(newData);
    }


    /*
    if(fork() == 0) {
        sleep(2);
        close(fd[0]);
        kill(parentPID, SIGUSR1);
        write(fd[1], "Inactivity detected!", 20);
        close(fd[1]);
        return 0;
    }
    else {
        int save_stdin = dup(STDIN_FILENO);
        close(fd[1]);
        signal(SIGUSR1, signalhandler);

        scanf("%s", text);
        printf("%s\n", text);
        
        printf("Restore stdin\n");
        dup2(save_stdin, STDIN_FILENO);
        text[0]=0;

        read(STDIN_FILENO, text, 20);

        printf("%s\n", text);
        wait(0);

    }

    */

    return 0;

}