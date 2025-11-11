#include <stdio.h>
#include <stdlib.h>
#include <sys/mann.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

void matrixMult(int** result, int** first, int** second, int left, int right);

int main(int argc, char** argv) {
    int par_id = atoi(argv[0]);
    int par_count = atoi(argv[1]);
    char* operations[] = {"MAB", "BMA", "MBA"};

    if(par_id == 0) {
        //Initialize all three matrices using shm

        //Send to others

        //Operation

        //Gather
    }

    //Other processes
    //wait for initialization


}

void matrixMult(int** result, int** first, int** second, int left, int right) {
    for(int i = left, i < right; i++) {

    }

    return;
}