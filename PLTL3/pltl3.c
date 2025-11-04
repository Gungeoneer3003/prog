#define _USE_MATH_DEFINES
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/time.h>

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

void imageProcess(int i, int n, LONG w, LONG h, int rwb, char* data, char* newData) {
    for (int y = i * h / n; y < (i + 1) * h / n; y++)
    {
        for (int x = 0; x < w; x++)
        {
            int c = x * 3 + y * rwb; // c for cursor

            //Switch case for each process's task
            if (i == 0) 
            {
                newData[c] = data[c];
                newData[c + 1] = 0;
                newData[c + 2] = data[c + 2];
            }
            else if (i == 1)
            {
                newData[c] = 0;
                newData[c + 1] = data[c+1];
                newData[c + 2] = data[c + 2];
            }
            else if (i == 2)
            {
                newData[c] = data[c];
                newData[c + 1] = data[c+1];
                newData[c + 2] = 255;
            }
            else if (i == 3) 
            {
                newData[c] = 255;
                newData[c + 1] = data[c+1];
                newData[c + 2] = data[c + 2];
            }
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
    BITMAPINFOHEADER fih;

    fread(&fh.bfType, 2, 1, f);
    fread(&fh.bfSize, 4, 1, f);
    fread(&fh.bfReserved1, 2, 1, f);
    fread(&fh.bfReserved2, 2, 1, f);
    fread(&fh.bfOffBits, 4, 1, f);

    fread(&fih, sizeof(fih), 1, f);

    char* data = (char*)mmap(NULL,fih.biSizeImage,PROT_READ|PROT_WRITE,MAP_SHARED|MAP_ANONYMOUS,-1,0);
    fread(data, fih.biSizeImage, 1, f);
    char* newData = (char*)mmap(NULL,fih.biSizeImage,PROT_READ|PROT_WRITE,MAP_SHARED|MAP_ANONYMOUS,-1,0);

    fclose(f);

    //Find real width bytes
    int byteWidth = fih.biWidth * fih.biBitCount / 8;
    int padding = 4 - byteWidth % 4;
    if(padding == 4)
        padding = 0;
    int rwb = byteWidth + padding;

    LONG w = fih.biWidth;
    LONG h = fih.biHeight;

    //Start time
    struct timeval tv1, tv2;
    gettimeofday(&tv1, NULL);

    //Perform operation
    while (i > 0) {
        int pid = fork();

        if(pid == 0) {
            printf("Child starts with i = %d \n", i);
            imageProcess(i, n, w, h, rwb, data, newData);
            printf("Child Ends\n");
            return 0;
        }
        i--;
    }

    //The parent does the first chunk
    printf("Parent Starts\n");
    imageProcess(i, n, w, h, rwb, data, newData);
    printf("Parent Ends Here\n");
    while(wait(0) != -1);

    //End time
    gettimeofday(&tv2, NULL);
    long long timing = (tv2.tv_sec - tv1.tv_sec) * 1000000LL + (tv2.tv_usec - tv1.tv_usec);
    printf("The operation took %lld microseconds.\n", timing);

    //Store results in output directory
    FILE* secondFile = fopen(output, "wb");
    
    fwrite(&fh.bfType, 2, 1, secondFile);
    fwrite(&fh.bfSize, 4, 1, secondFile);
    fwrite(&fh.bfReserved1, 2, 1, secondFile);
    fwrite(&fh.bfReserved2, 2, 1, secondFile);
    fwrite(&fh.bfOffBits, 4, 1, secondFile);

    fwrite(&fih, sizeof(fih), 1, secondFile);

    fwrite(newData, fih.biSizeImage, 1, secondFile);

    fclose(secondFile);

    munmap(data, fih.biSizeImage);
    munmap(newData, fih.biSizeImage);
    return 0;
}