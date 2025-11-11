#include <stdio.h>
#include <stdlib.h>
#include <sys/mann.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>

int main(int argc, char** argv) {
    // int processNum = atoi(argv[0]);
    // int totalProcesses = atoi(argv[1]);
    printf("Start\n");
    char* operations[] = {"MAB", "BMA", "MBA"};
    int c = 3;

    //Open
    int fdMatrix = shm_open("data", O_CREAT | O_RDWR, 0777);
    ftruncate(fdMatrix, sizeof(int) * c * c);
    int **arr = (int**)mmap(NULL,c * sizeof(int*),PROT_READ|PROT_WRITE,MAP_SHARED,fdMatrix,0);
    for(int i = 0; i < c; i++) {
        arr[i] = (int*)mmap(NULL,c * sizeof(int),PROT_READ|PROT_WRITE,MAP_SHARED,fdMatrix,0);
    }

    //Operation
    arr[c-1][c-1] = 21;

    //Close
    close(fdMatrix);
    for(int i = 0; i < c; i++) {
        munmap(arr[i], c * sizeof(int));
    }
    munmap(arr, c * sizeof(int*));

    
    printf("%d", sizeof(operations));

    // printf("Test, I am process %d %d\n", processNum, totalProcesses);
    // sleep(10);
    // printf("Process %d done sleeping.\n", processNum);
    return 0;
}