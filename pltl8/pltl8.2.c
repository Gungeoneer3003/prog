#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <sys/mman.h>

void gather(int* flagArr, int id, int count);
/*
    newArgv[0] = "2";
    newArgv[1] = "2";
    newArgv[2] = "Hello";
    newArgv[3] = processStr;
    newArgv[4] = countStr;
    newArgv[5] = NULL;
*/

const char* arg1 = "2";
const char* arg2 = "2";
const char* printable = "Hello";
const char* processnum = "0";
const char* processtotal = "4";

int main(int argc, char** argv) {

    // argv[0] = arg1;
    // argv[1] = arg2;
    // argv[2] = printable;
    // argv[3] = processnum;
    // argv[4] = processtotal;

    int x = atoi(argv[0]) * atoi(argv[1]);
    int n = atoi(argv[3]);
    int count = atoi(argv[4]);

    printf("Starting process %d of %d\n", n, count);

    int fdFlag = -1;
    int* flag;

    if(n == 0) {
        //Initialize flag arr
        fdFlag = shm_open("flag", O_CREAT | O_RDWR, 0777);
        ftruncate(fdFlag, sizeof(int) * (count+1));
        flag = (int*)mmap(0, sizeof(int) * (count + 1), PROT_READ | PROT_WRITE, MAP_SHARED, fdFlag, 0);

        for(int i = 0; i < count + 1; i++) 
            flag[i] = 0;
    }
    else {
        while(fdFlag == -1) {
            fdFlag = shm_open("flag", O_RDWR, 0777);
            if (fdFlag == -1)
                sleep(1);
        }

        flag = (int*)mmap(0, sizeof(int) * count, PROT_READ | PROT_WRITE, MAP_SHARED, fdFlag, 0);
    }

   
    gather(flag, n, count);
    
    
    for(int i = 0; i < x; i++) {
        printf("%s %d (index: %d)\n", argv[2], n, i);
        gather(flag, n, count);       
    }

    close(fdFlag);
    munmap(flag, sizeof(int)*count);
    if(n = 0)
        shm_unlink("flag");
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
