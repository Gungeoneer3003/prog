#define _USE_MATH_DEFINES
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/time.h>
#include <fcntl.h>

typedef unsigned short WORD;
typedef unsigned int DWORD;
typedef unsigned int LONG;
struct tagBITMAPFILEHEADER
{
WORD bfType; //specifies the file type
DWORD bfSize; //specifies the size in bytes of the bitmap file
WORD bfReserved1; //reserved; must be 0
WORD bfReserved2; //reserved; must be 0
DWORD bfOffBits; //species the offset in bytes from the bitmapfileheader to the bitmap bits
} typedef BITMAPFILEHEADER;
struct BITMAPINFOHEADER
{
DWORD biSize; //specifies the number of bytes required by the struct
LONG biWidth; //specifies width in pixels
LONG biHeight; //species height in pixels
WORD biPlanes; //specifies the number of color planes, must be 1
WORD biBitCount; //specifies the number of bit per pixel
DWORD biCompression;//spcifies the type of compression
DWORD biSizeImage; //size of image in bytes
LONG biXPelsPerMeter; //number of pixels per meter in x axis
LONG biYPelsPerMeter; //number of pixels per meter in y axis
DWORD biClrUsed; //number of colors used by th ebitmap
DWORD biClrImportant; //number of colors that are important
} typedef BITMAPINFOHEADER;


/*

for (int y = i * h / n; y < (i + 1) * h / n; y++)
    {
        for (int x = 0; x < w; x++)
        {
*/

void imageProcess(LONG w, LONG h, int rwb, char* data, char* newData) {
    for (int y = 0; y < h; y++)
    {
        for (int x = 3* w / 4; x < w; x++)
        {
            int c = x * 3 + y * rwb; // c for cursor

            int avg = data[c] + data[c+1] + data[c+2];
            avg /= 3;

            newData[c] = avg;
            newData[c + 1] = avg;
            newData[c + 2] = avg;
        }
    }
}

int main() {
    char* output = "output.bmp";
    
    int fdFIH = shm_open("FH", O_CREAT | O_RDWR, 0777);
    ftruncate(fdFIH, sizeof(BITMAPINFOHEADER));
    BITMAPINFOHEADER* fih = (BITMAPINFOHEADER*)mmap(NULL, sizeof(BITMAPINFOHEADER),PROT_READ|PROT_WRITE,MAP_SHARED, fdFIH, 0);

    int fdDATA = shm_open("data", O_RDWR, 0777);
    ftruncate(fdDATA, fih->biSizeImage);
    char* data = (char*)mmap(NULL,fih->biSizeImage,PROT_READ|PROT_WRITE,MAP_SHARED,fdDATA,0);

    int fd = shm_open(output, O_RDWR, 0777);
    if (fd == -1) {
        perror("Didn't work");
        exit(1);
    }
    if (ftruncate(fd, fih->biSizeImage) == -1) {
        perror("failed to truncate");
        exit(1);
    }
    char* newData = (char*)mmap(0, fih->biSizeImage, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);

    //Find real width bytes
    int byteWidth = fih->biWidth * 3;
    int padding = 4 - byteWidth % 4;
    if(padding == 4)
        padding = 0;
    int rwb = byteWidth + padding;

    LONG w = fih->biWidth;
    LONG h = fih->biHeight;

    //Do the work
    imageProcess(w, h, rwb, data, newData);

    //Set up flag for completion of prog 1
    int fd2 = shm_open("flag", O_RDWR, 0777);
    while(fd2 == -1) {
        fd2 = shm_open("flag", O_RDWR, 0777);
        sleep(1);
    }
    ftruncate(fd2, 12);
    int* flag = (int*)mmap(0, 12, PROT_READ | PROT_WRITE, MAP_SHARED, fd2, 0);

    flag[2] += 1;

    
    close(fdDATA);
    close(fdFIH);
    close(fd);
    close(fd2);
    munmap(data, fih->biSizeImage);
    munmap(newData, fih->biSizeImage);
    return 0;
}