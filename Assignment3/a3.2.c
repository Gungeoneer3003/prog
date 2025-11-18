#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#define SIZE 10

//Prototypes
void matrixMult(int** arr, int left, int right);
void operationMatch(int** arr, int* first, int* second, int* third, char* oper);
void gather(int* flagArr, int id, int count);
void printMatrix(int* arr);
float det(const int *matrix, int cols, int rows);
int *minor(const int *matrix, int skip_row, int skip_col, int cols, int rows);

int main(int argc, char** argv) {
    int par_id = atoi(argv[0]);
    int par_count = atoi(argv[1]);
    // int par_id = 0;
    // int par_count = 1;

    //printf("Process %d of %d starting (%d).\n", par_id, par_count, getpid());
    
    int** eq = {NULL, NULL, NULL};
    char* operations[] = {"MAB", "BMA", "MBA"};
    int operCount = sizeof(operations) / sizeof(operations[0]);
    
    //Initialize var's consistent to both
    int fdM, fdA, fdB, fdFlag;
    int *M, *A, *B, *flag;
    int **arr = malloc(3 * sizeof(int*)); //This will contain the in order eq (e.g. M = A * B)

    
    if(par_id == 0) {
        //Initialize all four arrays using shm
        fdM = shm_open("M", O_CREAT | O_RDWR, 0777);
        ftruncate(fdM, SIZE * SIZE * sizeof(int));
        M = (int*)mmap(NULL,SIZE * SIZE * sizeof(int),PROT_READ|PROT_WRITE,MAP_SHARED,fdM,0);

        fdA = shm_open("A", O_CREAT | O_RDWR, 0777);
        ftruncate(fdA, SIZE * SIZE * sizeof(int));
        A = (int*)mmap(NULL,SIZE * SIZE * sizeof(int),PROT_READ|PROT_WRITE,MAP_SHARED,fdA,0);

        fdB = shm_open("B", O_CREAT | O_RDWR, 0777);
        ftruncate(fdB, SIZE * SIZE * sizeof(int));
        B = (int*)mmap(NULL,SIZE * SIZE * sizeof(int),PROT_READ|PROT_WRITE,MAP_SHARED,fdB,0);
        
        //Initialize flag arr
        fdFlag = shm_open("flag", O_CREAT | O_RDWR, 0777);
        ftruncate(fdFlag, sizeof(int) * (par_count+1));
        flag = (int*)mmap(0, sizeof(int) * (par_count+1), PROT_READ | PROT_WRITE, MAP_SHARED, fdFlag, 0);

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
        //printf("Now setting up fd's\n");
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

        //Set up arrays
        M = (int*)mmap(NULL,SIZE * SIZE * sizeof(int),PROT_READ|PROT_WRITE,MAP_SHARED,fdM,0);
        A = (int*)mmap(NULL,SIZE * SIZE * sizeof(int),PROT_READ|PROT_WRITE,MAP_SHARED,fdA,0);
        B = (int*)mmap(NULL,SIZE * SIZE * sizeof(int),PROT_READ|PROT_WRITE,MAP_SHARED,fdB,0);
        flag = (int*)mmap(0, sizeof(int) * (par_count+1), PROT_READ | PROT_WRITE, MAP_SHARED, fdFlag, 0);

        //printf("Done setting up fd's\n");
    }

    //Define bounds, which will be different for each
    int left = par_id * SIZE / par_count;
    int right = (par_id + 1) * SIZE / par_count;  

    //printf("Here %d has %d to %d\n", par_id, left, right);

    gather(flag, par_id, par_count);

    struct timeval tv1, tv2;
    if(par_id == 0) {
        gettimeofday(&tv1, NULL);
    }

    for(int i = 0; i < operCount; i++) {
        
        //Initial Set up
        operationMatch(arr, M, A, B, operations[i]);
        if(par_id == 0) {
            for(int u = 0; u < SIZE; u++) {
                for(int j = 0; j < SIZE; j++) {
                    arr[0][u * SIZE + j] = 0;
                }
            }
            
        }

        //printf("Process %d waiting at gather before operation %s\n", par_id, operations[i]);
        gather(flag, par_id, par_count);

        //printf("Process %d starting operation %s\n", par_id, operations[i]);
        matrixMult(arr, left, right);
        //printf("Process %d finished operation %s\n", par_id, operations[i]);

        gather(flag, par_id, par_count);
        //printf("Past the %d gather\n", i);

        if(par_id == 0)
            printMatrix(arr[0]);
    }    


    gather(flag, par_id, par_count);

    if(par_id == 0) {
        gettimeofday(&tv2, NULL);
        long long timing = (tv2.tv_sec - tv1.tv_sec) * 1000000LL + (tv2.tv_usec - tv1.tv_usec);
        printf("The operation took %lld microseconds.\n", timing);
    }

    if(par_id == 0) {
        printf("Now attempting to find the determinate.\n");
        printf("Determinate is %f\n", det(M, SIZE, SIZE));
    }

    
    
    
    //Close
    free(arr);
    close(fdFlag);
    munmap(flag, sizeof(int) * (par_count+1));
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
            for(int i = left; i < right; i++)
                ans[k * SIZE + u] += first[k * SIZE + i] * second[i * SIZE + u];
        }
    }

    return;
}

void gather(int* flagArr, int id, int count) {
    int synchid = flagArr[count] + 1;
    flagArr[id] = synchid;

    while(1) {
        //sleep(1);
        int L = 1;
        for(int i = 0; i < count; i++) {
            if(flagArr[i] < synchid)
                L = 0;
        }

        if(L) {
            flagArr[count] = synchid;
            break;
        }
    }
}

void printMatrix(int* arr) {
    for(int i = 0; i < SIZE; i++) {
        for(int j = 0; j < SIZE; j++) {
            printf("%d ", arr[j + i * SIZE]);
        }
        printf("\n");
    }
    printf("\n");
}

float det(const int *matrix, int cols, int rows) {
    if(rows == 1 && cols == 1) {
        return matrix[0];
    }

    if(rows == 2 && cols == 2) {
        return matrix[0] * matrix[3] - matrix[1] * matrix[2];
    }
    

    float ans = 0;

    for(int j = 0; j < cols; j++) {
        int sign = (j % 2 == 0) ? 1 : -1;
        float coeff = sign * matrix[j];
        int *m = minor(matrix, 0, j, cols, rows);
        float subdet = det(m, cols - 1, rows - 1);
        ans += coeff * subdet;
        free(m);
    }

    return ans;
}


int *minor(const int *matrix, int skip_row, int skip_col, int cols, int rows) {
    int minor_size = (cols - 1) * (rows - 1);
    int *minor_matrix = malloc(minor_size * sizeof(int));
    int index = 0;
    for(int i = 0; i < rows; i++) {
        for(int j = 0; j < cols; j++) {
            if(i == skip_row || j == skip_col) {
                continue;
            }
            minor_matrix[index++] = matrix[i * cols + j];
        }
    }
    return minor_matrix;
}