#include <fcntl.h>

int main() {
    shm_unlink("data");
    shm_unlink("FH");
    shm_unlink("flag");
    shm_unlink("output.bmp");
}