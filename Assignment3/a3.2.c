#include <stdio.h>
#include <stdlib.h>
#include <sys/mann.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#define SIZE 3


//Prototypes
void matrixMult(int** arr, int left, int right);
void operationMatch(int** arr, int* first, int* second, int* third, char* oper);
void gather(int* flagArr, int id, int count);
void printMatrix(int* arr);

int main(int argc, char** argv) {
    int par_id = atoi(argv[0]);
    int par_count = atoi(argv[1]);
    
    int*** eq = {NULL, NULL, NULL};
    char* operations[] = {"MAB", "BMA", "MBA"};
    int operCount = sizeof(operations) / sizeof(operations[0]);
    
    //Initialize var's consistent to both
    int fdM, fdA, fdB, fdFlag;
    int *M, *A, *B, *flag;
    int **arr; //This will contain the in order eq (e.g. M = A * B)

    
    if(par_id == 0) {
        //Initialize all four arrays using shm
        fdM = shm_open("M", O_CREAT | O_RDWR, 0777);
        ftruncate(fdMatrix, SIZE * SIZE * sizeof(int));
        M = (int*)mmap(NULL,SIZE * SIZE * sizeof(int),PROT_READ|PROT_WRITE,MAP_SHARED,fdMatrix,0);

        fdA = shm_open("A", O_CREAT | O_RDWR, 0777);
        ftruncate(fdMatrix, SIZE * SIZE * sizeof(int));
        A = (int*)mmap(NULL,SIZE * SIZE * sizeof(int),PROT_READ|PROT_WRITE,MAP_SHARED,fdMatrix,0);

        fdB = shm_open("B", O_CREAT | O_RDWR, 0777);
        ftruncate(fdMatrix, SIZE * SIZE * sizeof(int));
        B = (int*)mmap(NULL,SIZE * SIZE * sizeof(int),PROT_READ|PROT_WRITE,MAP_SHARED,fdMatrix,0);
        
        //Initialize flag arr
        fdFlag = shm_open("flag", O_CREAT | O_RDWR, 0777);
        ftruncate(fdFlag, sizeof(int) * par_count);
        flag = (int*)mmap(0, sizeof(int) * count, PROT_READ | PROT_WRITE, MAP_SHARED, fdFlag, 0);

        //Write random elements to each starting array
        for(int i = 0; i < SIZE; i++) {
            for(int j = 0; j < SIZE; j++) {
                A[i*SIZE + j] = rand() % 10;
                B[i*SIZE + j] = rand() % 10;
                M[i*SIZE + j] = 0;
            }
        }

        //(May not be necessary)
        //Set all flags to 0
        for (int i = 0; i < par_count; i++) 
            flag[i] = 0;
        

        printMatrix(A);
        printMatrix(B);
    }
    else {
        //Set up what the variables would otherwise be
        fdM = -1;
        fdA = -1;
        fdB = -1;
        fdFlag = -1;

        //Wait for intialization
        while(fdM == -1) {
            fdM = shm_open("M", O_RDWR, 0777);
            if (fdM == -1)
                sleep(1);
        }

        while(fdA == -1) {
            fdA = shm_open("A", O_RDWR, 0777);
            if (fdA == -1)
                sleep(1);
        }

        while(fdB == -1) {
            fdB = shm_open("B", O_RDWR, 0777);
            if (fdA == -1)
                sleep(1);
        }

        while(fdFlag == -1) {
            fdFlag = shm_open("flag", O_RDWR, 0777);
            if (fdFlag == -1)
                sleep(1);
        }
    }

    


    //Define bounds, which will be different for each
    int left = par_id * SIZE / par_count;
    int right = (par_id + 1) * SIZE / par_count;  

    for(int i = 0; i < operCount; i++) {
        flag[par_id] = 0;
        operationMatch(arr, M, A, B, operations[i]);

        matrixMult(arr, left, right);

        gather(flag, par_id, par_count);
        printMatrix(arr[0]);
    }

    

    //Close
    close(fdFlag);
    munmap(flag, sizeof(int) * par_count);
    close(fdM);
    munmap(M, SIZE * SIZE * sizeof(int));
    close(fdA);
    munmap(A, SIZE * SIZE * sizeof(int));
    close(fdB);
    munmap(B, SIZE * SIZE * sizeof(int));

    if(par_id == 0) {
        shm_unlink("M");
        shm_unlink("A");
        shm_unlink("B");
        shm_unlink("flag");
    }

    return 0;
}

//Assuming no faulty strings in this function
//A faulty string would be one such has bad variables
void operationMatch(int** arr, int* M, int* A, int* B, char* oper) {
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
void matrixMult(int** arr, int left, int right) {
    int* ans = arr[0];
    int* first = arr[1];
    int* second = arr[2];
    
    //Each program should take up one row through this
    for(int k = 0; k < SIZE; k++) {
        for(int u = 0; u < SIZE; u++) {
            for(int i = 0; i < SIZE; i++)
                ans[k * SIZE + u] += first[i * SIZE + u] * B[k * SIZE + i];
        }
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

void printMatrix(int* arr) {
    for(int i = 0; i < SIZE; i++) {
        for(int j = 0; j < SIZE; j++) {
            printf("%d ", arr[j + i * SIZE]);
        }
        printf("\n");
    }
}