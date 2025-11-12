#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <sys/mman.h>

int main() {
    printf("Start\n");

    int c = 3;
    int fdMatrix = shm_open("data", O_RDWR, 0777);
    int *arr = (int*)mmap(NULL,c * c * sizeof(int),PROT_READ|PROT_WRITE,MAP_SHARED,fdMatrix,0);

    printf("Value at arr[2][2]: %d\n", arr[(c-1) * c + (c-1)]);

    close(fdMatrix);
    munmap(arr, c * c * sizeof(int));
    printf("Done.\n");
}