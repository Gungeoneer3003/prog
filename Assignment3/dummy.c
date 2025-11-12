#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <sys/mman.h>

int main(int argc, char** argv) {
    // int processNum = atoi(argv[0]);
    // int totalProcesses = atoi(argv[1]);
    printf("Start\n");
    // int* a[] = {{1},{4}, {7, 8, 9, 10, 11, 12, 13, 14, 15, 16}};
    // printf("Sizeof a: %d\n", sizeof(a));

    char* operations[] = {"MAB", "BMA", "MBA"};
    int c = 3;

    //Open
    int fdMatrix = shm_open("data", O_CREAT | O_RDWR, 0777);
    ftruncate(fdMatrix, c * c * sizeof(int));
    int *arr = (int*)mmap(NULL,c * c * sizeof(int),PROT_READ|PROT_WRITE,MAP_SHARED,fdMatrix,0);

    //Write the last element
    arr[(c-1) * c + (c-1)] = 21;

    printf("Value at arr[2][2]: %d\n", arr[(c-1) * c + (c-1)]);

    //Close
    close(fdMatrix);
    munmap(arr, c * c * sizeof(int));

    
    printf("%d", sizeof(operations));

    // printf("Test, I am process %d %d\n", processNum, totalProcesses);
    // sleep(10);
    // printf("Process %d done sleeping.\n", processNum);
    return 0;
}