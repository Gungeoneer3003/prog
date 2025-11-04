#include <unistd.h>
#include <signal.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <fcntl.h>
#include <errno.h>

int main() {
    sleep(1);
    const char* filename = "abc";
    printf("here\n");
    int fd = -1;
    while(fd == -1) 
        fd = shm_open(filename, O_RDWR, 0777);

    printf("past that\n");

    char *d = mmap(0, 100, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);

    printf("%s\n", d);

    close(fd);
    munmap(d, 100);
    shm_unlink(filename);
    return 0;
}