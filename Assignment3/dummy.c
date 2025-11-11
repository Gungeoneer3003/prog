#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char** argv) {
    // int processNum = atoi(argv[0]);
    // int totalProcesses = atoi(argv[1]);
    char* operations[] = {"MAB", "BMA", "MBA"};

    printf("%d", sizeof(operations));

    // printf("Test, I am process %d %d\n", processNum, totalProcesses);
    // sleep(10);
    // printf("Process %d done sleeping.\n", processNum);
    return 0;
}