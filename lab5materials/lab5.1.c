#include <unistd.h>
#include <signal.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <fcntl.h>

int main() {
    const char* filename = "abc";
    char input[100];

    int x = read(STDIN_FILENO, &input, 100);
    input[x-1] = 0;

    printf("%s\n", input);

    int fd = shm_open(filename, O_CREAT | O_RDWR, 0777);
    if (fd == -1) {
        perror("Didn't work");
        exit(1);
    }
    if (ftruncate(fd, 100) == -1) {
        perror("failed to truncate");
        exit(1);
    }
    char* d = mmap(0, 100, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);

    for(int i = 0; i < x; i++)
        d[i] = input[i];

    printf("%s\n", d);

    close(fd);
    
    //This command throws out the file in shm
    //shm_unlink(filename); 

    munmap(d, 100);
    return 0;
}