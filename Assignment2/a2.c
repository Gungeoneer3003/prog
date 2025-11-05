#include <stdio.h>
#include <stdlib.h>
#include <string.h>
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

int getColor(char* data, int rwb, int rgb, int x, int y) {
    int c = x * 3 + y * rwb;

    return data[c + rgb];
}

int interpolate(char* data, int rwb, int rgb, float x, float y) {
    float dx = x - trunc(x);
    float dy = y - trunc(y);

    //Bounds, which must be integers
    int x1 = x - dx;        
    int x2 = x + (1-dx);
    int y1 = y - dy;
    int y2 = y + (1-dy);

    //Colors, which also must be integers
    int left_upper = get_color(data, rwb, rgb,x1,y2);
    int right_upper = get_color(data, rwb, rgb,x2,y2);
    int left_lower = get_color(data, rwb, rgb,x1,y1);
    int right_lower = get_color(data, rwb, rgb,x2,y1);

    //Interpolate 
    float left = left_upper * (1 - dy) + left_lower * dy;
    float right = right_upper * (1 - dy) + right_lower * dy;
    int result = left * (1 - dx) + right * dx;

    return result;
}

void imageProcess(int i, int n, LONG w, LONG h, int rwb, char* data, char* newData) {
    for (int x = i * w / n; x < (i + 1) * w / n; x++)
    {
        for (int y = 0; y < h; y++)
        {
            int c = x * 3 + y * rwb; // c for cursor

            newData[c] = data[c];
            newData[c + 1] = data[c + 1];
            newData[c + 2] = data[c + 2];
        }
    }
}


int main(int argc, char** argv) {
    //Hard coded inputs 
    float ratio = 0.3;
    char* input1 = "lion.bmp";
    char* input2 = "flowers.bmp";
    int n = 1;
    char* output = "result.bmp";


    //Checking inputs
    if(ratio < 0 | ratio > 1) {
        printf("The ratio is invalid. Try again\n");
        return 0;
    }

    if (n < 1 | n > 4) {
        printf("Invalid processor count. Try again\n");
        return 0;
    }

    //Handle input1
    FILE* f1 = fopen(input1, "rb");
    if(f1 == NULL) {
        perror("Failed to open the file.");
        return 1;
    }

    //Take BMP Input1
    BITMAPFILEHEADER fh1;
    BITMAPINFOHEADER fih1;

    fread(&fh1.bfType, 2, 1, f1);
    fread(&fh1.bfSize, 4, 1, f1);
    fread(&fh1.bfReserved1, 2, 1, f1);
    fread(&fh1.bfReserved2, 2, 1, f1);
    fread(&fh1.bfOffBits, 4, 1, f1);

    fread(&fih1, sizeof(fih1), 1, f1);

    char* data1 = (char*)mmap(NULL,fih1.biSizeImage,PROT_READ|PROT_WRITE,MAP_SHARED|MAP_ANONYMOUS,-1,0);
    fread(data1, fih.biSizeImage, 1, f1);

    fclose(f1);

    //Find real width bytes for the first
    int byteWidth = fih.biWidth * 3;
    int padding = 4 - byteWidth % 4;
    if(padding == 4)
        padding = 0;
    int rwb1 = byteWidth + padding;

    LONG w1 = fih.biWidth;
    LONG h1 = fih.biHeight;

    //Handle input2

    //Create result image memory
    char* newData = (char*)mmap(NULL,fih1.biSizeImage,PROT_READ|PROT_WRITE,MAP_SHARED|MAP_ANONYMOUS,-1,0);

    //Handle sizing

    //Process Image
    imageProcess(0, 1, w1, h1, rwb1, data, newData);

    //Create bmp file
    FILE* lastFile = fopen(output, "wb");
    
    fwrite(&fh1.bfType, 2, 1, lastFile);
    fwrite(&fh1.bfSize, 4, 1, lastFile);
    fwrite(&fh1.bfReserved1, 2, 1, lastFile);
    fwrite(&fh1.bfReserved2, 2, 1, lastFile);
    fwrite(&fh1.bfOffBits, 4, 1, lastFile);

    fwrite(&fih1, sizeof(fih1), 1, lastFile);

    fwrite(newData, fih.biSizeImage, 1, lastFile);

    fclose(lastFile);


    //Close up shop
    munmap(data1, fih1.biSizeImage);
    //munmap(data2, fih2.biSizeImage);
    munmap(newData, fih1.biSizeImage); 
    return 0;
}