#include <stdio.h>
#include <unistd.h>
#include <sys/mman.h>

int main(int argc, char **argv) {
    //Insert time start here
    int x = 0;
    int *i = (int*)mmap(NULL,4,PROT_READ|PROT_WRITE,MAP_SHARED|MAP_ANONYMOUS,-1,0);
    int pid = fork();
    
    //If you want more than one kid
    /*
    if(pid == 0) {
        printf("kid %d\n", getpid());
        return 0;
    }

    pid = fork();
    if(pid == 0) {
        printf("kid %d\n", getpid());
        return 0;
    }
    */

    if(pid == 0) {
        printf("kid %d\n", getpid());
        //Sleep(1)
        x = 100;
        *i = 99;
        return 0;
    }
    //Technically the else isn't necessary since we kill the kid by returning, 
    //but you get the same idea, the flow does not continue past the kid.
    else {
        printf("parent %d\n", getpid());
        //sleep(1); Only 1s pass if placed before wait(0)
        wait(0);
        //Insert time stop here

        printf("%d\n", x);
        printf("%d\n", *i);
        printf("both\n");
        munmap(i,4);
    }
}