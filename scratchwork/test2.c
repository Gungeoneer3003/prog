#include <stdio.h>
#include <sys/mman.h>

int main() {
    int c = 3;
    int **arr = (int**)mmap(NULL,c * sizeof(int*),PROT_READ|PROT_WRITE,MAP_SHARED|MAP_ANONYMOUS,-1,0);
    for(int i = 0; i < c; i++) {
        arr[i] = (int*)mmap(NULL,c * sizeof(int),PROT_READ|PROT_WRITE,MAP_SHARED|MAP_ANONYMOUS,-1,0);
    }

    for(int i = 0; i < c; i++) {
        for(int j = 0; j < c; j++) { 
            arr[i][j] = i + j + 1;
            printf("%d ", arr[i][j]);
        }
        printf("\n");
    }

    for(int i = 0; i < c; i++) {
        munmap(arr[i], c * sizeof(int));
    }
    munmap(arr, c * sizeof(int*));
}