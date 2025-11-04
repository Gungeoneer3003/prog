#define _USE_MATH_DEFINES
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <unistd.h>
#include <sys/mman.h>

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

void imageProcess(int i, int n, LONG w, LONG h, int rwb, float alpha, DWORD max, char* data, char* newData) {
    for (int x = i * w / n; x < (i + 1) * w / n; x++)
    {
        for (int y = 0; y < h; y++)
        {
            int c = x * 3 + y * rwb; // c for cursor

            float x_r = x - w / 2.0f;
            float y_r = y - h / 2.0f;

            float x_ot = x_r * cos(alpha) - y_r * sin(alpha);
            float y_ot = x_r * sin(alpha) + y_r * cos(alpha);
            int newX = x_ot + w / 2.0f;
            int newY = y_ot + h / 2.0f;
            if (newX >= w || newY >= h)
                continue;

            int newC = newX * 3 + newY * rwb;
            if (newC > max || newC < 0)
                continue;

            newData[c] = data[newC];
            newData[c + 1] = data[newC + 1];
            newData[c + 2] = data[newC + 2];
        }
    }
}

int main(int argc, char** argv) {
    //Take command line arguments
    /*
    if(argc != 5) {
        printf("Invalid input count. Try again.\n");
        return 0;
    }

    char* input = argv[1];
    float alpha = atof(argv[2]); //rotation angle in radians
    int n = atoi(argv[3]); //Count of processes
    char* output = argv[4];

    if(n < 1 || n > 4) {
        printf("Invalid count of processors, please try again.\n");
        return 0;
    }
    */

    char* input = "PLTL2.bmp";
    float alpha = M_PI/6.0; //rotation angle in radians
    int n = 2; //Count of processes
    char* output = "output.bmp";

    alpha *= -1;
    
    int i = n - 1;

    //Take in file
    /*
    //Insert proper file intake using mmap
    */

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
    DWORD max = fih.biSizeImage;

    //Perform operation
    while (i > 0) {
        int pid = fork();

        if(pid == 0) {
            printf("Child starts\n");
            imageProcess(i, n, w, h, rwb, alpha, max, data, newData);
            printf("Child Ends\n");
            return 0;
        }
        i--;
    }

    //The parent does the first chunk
    printf("Parent Starts\n");
    imageProcess(i, n, w, h, rwb, alpha, max, data, newData);
    printf("Parent Ends Here\n");
    while(wait(0) != -1);

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