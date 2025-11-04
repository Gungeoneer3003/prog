#include <unistd.h>
#include <signal.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/wait.h>
#include <sys/types.h>

int fd[2];
void signalhandler(int i)
{
    dup2(fd[0], STDIN_FILENO);
    printf("Inactivity detected!\n");
}

int main() {
    pipe(fd);
    int parentPID = getpid();
    char text[100];

    
    if(fork() == 0) {
        sleep(2);
        close(fd[0]);
        kill(parentPID, SIGUSR1);
        write(fd[1], "Inactivity detected!", 6);
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

    close(fd[0]);
    return 0;
}