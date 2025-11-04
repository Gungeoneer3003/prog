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
        for (int x = 0; x < w / 4; x++)
        {
            int c = x * 3 + y * rwb; // c for cursor

            newData[c] = data[c];
            newData[c + 1] = data[c+1];
            newData[c + 2] = 0;
        }
    }
}

int main() {
    char* input = "PLTL2.bmp";
    char* output = "output.bmp";
    int n = 1;
    int i = n - 1;

    FILE* f = fopen(input, "rb");
    if(f == NULL) {
        perror("Failed to open the file.");
        return 1;
    }

    //Take BMP Input
    BITMAPFILEHEADER fh;
    
    int fdFIH = shm_open("FH", O_CREAT | O_RDWR, 0777);
    ftruncate(fdFIH, sizeof(BITMAPINFOHEADER));
    BITMAPINFOHEADER* fih = (BITMAPINFOHEADER*)mmap(NULL, sizeof(BITMAPINFOHEADER),PROT_READ|PROT_WRITE,MAP_SHARED, fdFIH, 0);

    fread(&fh.bfType, 2, 1, f);
    fread(&fh.bfSize, 4, 1, f);
    fread(&fh.bfReserved1, 2, 1, f);
    fread(&fh.bfReserved2, 2, 1, f);
    fread(&fh.bfOffBits, 4, 1, f);

    fread(fih, sizeof(BITMAPINFOHEADER), 1, f);

    int fdDATA = shm_open("data", O_CREAT | O_RDWR, 0777);
    ftruncate(fdDATA, fih->biSizeImage);
    char* data = (char*)mmap(NULL,fih->biSizeImage,PROT_READ|PROT_WRITE,MAP_SHARED,fdDATA,0);
    fread(data, fih->biSizeImage, 1, f);

    int fd = shm_open(output, O_CREAT | O_RDWR, 0777);
    if (fd == -1) {
        perror("Didn't work");
        exit(1);
    }
    if (ftruncate(fd, fih->biSizeImage) == -1) {
        perror("failed to truncate");
        exit(1);
    }
    char* newData = (char*)mmap(0, fih->biSizeImage, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);

    fclose(f);

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
    int fd2 = shm_open("flag", O_CREAT | O_RDWR, 0777);
    ftruncate(fd2, 12);
    int* flag = (int*)mmap(0, 4 * 3, PROT_READ | PROT_WRITE, MAP_SHARED, fd2, 0);
    for(int i = 0; i < 3; i++)
        flag[i] = 0;

    while(1) {
        sleep(1);
        int L = 1;
        for(int i = 0; i < 3; i++) {
            if(flag[i] == 0)
                L = 0;
        }

        if(L)
            break;
    }

    //Image creation, to be deleted when prog 2 is created
    FILE* secondFile = fopen("out.bmp", "wb");
    
    fwrite(&fh.bfType, 2, 1, secondFile);
    fwrite(&fh.bfSize, 4, 1, secondFile);
    fwrite(&fh.bfReserved1, 2, 1, secondFile);
    fwrite(&fh.bfReserved2, 2, 1, secondFile);
    fwrite(&fh.bfOffBits, 4, 1, secondFile);

    fwrite(fih, sizeof(BITMAPINFOHEADER), 1, secondFile);

    fwrite(newData, fih->biSizeImage, 1, secondFile);

    fclose(secondFile);
    

    shm_unlink("flag");
    shm_unlink("output.bmp");
    close(fdDATA);
    close(fdFIH);
    close(fd);
    close(fd2);
    munmap(data, fih->biSizeImage);
    munmap(newData, fih->biSizeImage);
    return 0;
}