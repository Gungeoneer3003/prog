#include <stdio.h>
#include <stdlib.h>
#include <sys/mann.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#define SIZE 10


//Prototypes
void matrixMult(int** result, int** first, int** second, int left, int right);
int*** operationMatch(int*** arr, int** first, int** second, int** third, char* oper);
void gather(int* flagArr, int id, int count);

int main(int argc, char** argv) {
    int par_id = atoi(argv[0]);
    int par_count = atoi(argv[1]);
    
    int*** eq = {NULL, NULL, NULL};
    char* operations[] = {"MAB", "BMA", "MBA"};
    int operCount = sizeof(operations) / sizeof(operations[0]);

    //Initialize flag arr
    int fd2 = shm_open("flag", O_CREAT | O_RDWR, 0777);
    ftruncate(fd2, sizeof(int) * count);
    int* flag = (int*)mmap(0, sizeof(int) * count, PROT_READ | PROT_WRITE, MAP_SHARED, fd2, 0);
    for(int i = 0; i < count; i++)
        flag[i] = 0;


    if(par_id == 0) {
        //Initialize all three matrices using shm
        

        //Fill the first two matrices with random data
        for (int i = 0; i < SIZE; i++) {
            for (int j = 0; j < SIZE; j++) {
                A[i][j] = rand() % 10;  
            }
        }

        for (int i = 0; i < SIZE; i++) {
            for (int j = 0; j < SIZE; j++) {
                B[i][j] = rand() % 10;  
            }
        }

        //Send to others
        
    }
    else {
        //Wait for intialization
    }

    for(int i = 0; i < operCount; i++) {
        operationMatch(arr, M, A, B, operations[0]);

        

        gather(flag, par_id, par_count);
    }

    //Wrap up program
    if(par_id == 0) {

    }
}

//Assuming no faulty strings in this function
//A faulty string would be one such has bad variables
void operationMatch(int*** arr, int** M, int** A, int** B, char* oper) {
    //First Variable
    if(oper[0] == 'M')
        arr[0] = M;
    else if(oper[0] == 'A')
        arr[0] = A;
    else
        arr[0] = B;
        
    //Second Variable
    if(oper[1] == 'M')
        arr[1] = M;
    else if(oper[1] == 'A')
        arr[1] = A;
    else
        arr[1] = B;

    //Third variable
    if(oper[2] == 'M')
        arr[2] = M;
    else if(oper[2] == 'A')
        arr[2] = A;
    else
        arr[2] = B;
}

/*
arr
0 - result matrix
1 - first
2 - second
*/
void matrixMult(int*** arr, int left, int right) {
    for(int i = left, i < right; i++) {

    }

    return;
}

void gather(int* flagArr, int id, int count) {
    flagArr[id] = 1;

    while(1) {
        sleep(1);
        int L = 1;
        for(int i = 0; i < count; i++) {
            if(flag[i] == 0)
                L = 0;
        }

        if(L)
            break;
    }
}