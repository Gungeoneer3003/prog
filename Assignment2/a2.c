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

unsigned char getColor(char* data, int rwb, int rgb, int x, int y) {
    int c = x * 3 + y * rwb;

    return data[c + rgb];
}

unsigned char interpolate(char* data, int rwb, int rgb, float x, float y, LONG w, LONG h) {
    float dx = x - floor(x);
    float dy = y - floor(y);

    //Bounds, which must be integers
    int x1 = floor(x);      
    int x2 = ceil(x);
    if(x2 >= w) x2 = x1;

    int y1 = floor(y);
    int y2 = ceil(y);
    if(y2 >= h) y2 = y1;

    //Colors, which also must be integers
    unsigned char left_upper = getColor(data, rwb, rgb,x1,y2);
    unsigned char right_upper = getColor(data, rwb, rgb,x2,y2);
    unsigned char left_lower = getColor(data, rwb, rgb,x1,y1);
    unsigned char right_lower = getColor(data, rwb, rgb,x2,y1);

    //Interpolate 
    float left = left_upper * (dy) + left_lower * (1- dy);
    float right = right_upper * (dy) + right_lower * (1-dy);
    unsigned char result = left * (1-dx) + right * (dx);

    // if (result == 255)
    //     printf("Here at %f %f\n", x, y);

    return result;
}

void imageProcess(int i, int n, LONG w, LONG h, int rwb1, int rwb2, char* data1, char* data2, float ratio, float scaleX, float scaleY, char* newData, LONG w2, LONG h2) {
    for (int x = i * w / n; x < (i + 1) * w / n; x++)
    {
        for (int y = 0; y < h; y++)
        {
            int c = x * 3 + y * rwb1; // c for cursor

            unsigned char r2 = interpolate(data2, rwb2, 2, x * scaleX, y * scaleY, w2, h2);
            unsigned char g2 = interpolate(data2, rwb2, 1, x * scaleX, y * scaleY, w2, h2);
            unsigned char b2 = interpolate(data2, rwb2, 0, x * scaleX, y * scaleY, w2, h2);

            // if((r2 < 255 && r2 > 0) || (g2 < 255 && g2 > 0) || (b2 < 255 && b2 > 0))
            //     printf("Here\n");
            float firstBlue = ((unsigned char)data1[c]) * ratio;
            float firstGreen = ((unsigned char)data1[c+1]) * ratio;
            float firstRed = ((unsigned char)data1[c+2]) * ratio;

            float secondBlue = b2 * (1-ratio);
            float secondGreen = g2 * (1- ratio);
            float secondRed = r2 * (1-ratio);

            unsigned char blue = (unsigned char)(firstBlue + secondBlue);
            unsigned char green = (unsigned char)(firstGreen + secondGreen);
            unsigned char red = (unsigned char)(firstRed + secondRed);

            newData[c] = blue;
            newData[c + 1] = green;
            newData[c + 2] = red;
        }
    }
}


int main(int argc, char** argv) {
    //Terminal Inputs
    // if (argc < 6) {
    //     printf("Insufficient arguments, try again.");
    //     return 0;
    // }
    // char* input1 = argv[1];
    // char* input2 = argv[2];
    // float ratio = atof(argv[3]);
    // int n = atoi(argv[4]);
    // char* output = argv[5];

    //Hard coded inputs 
    float ratio = 0.3;
    char* input1 = "flowers.bmp";
    char* input2 = "Mario.bmp";
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

    FILE* f2 = fopen(input2, "rb");
    if(f2 == NULL) {
        fclose(f1);
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
    fread(data1, fih1.biSizeImage, 1, f1);

    fclose(f1);

    //Take BMP Input2
    BITMAPFILEHEADER fh2;
    BITMAPINFOHEADER fih2;

    fread(&fh2.bfType, 2, 1, f2);
    fread(&fh2.bfSize, 4, 1, f2);
    fread(&fh2.bfReserved1, 2, 1, f2);
    fread(&fh2.bfReserved2, 2, 1, f2);
    fread(&fh2.bfOffBits, 4, 1, f2);

    fread(&fih2, sizeof(fih2), 1, f2);

    char* data2 = (char*)mmap(NULL,fih2.biSizeImage,PROT_READ|PROT_WRITE,MAP_SHARED|MAP_ANONYMOUS,-1,0);
    fread(data2, fih2.biSizeImage, 1, f2);

    fclose(f2);

    //Figure out which of the two is biggest and swap 
    if(fih1.biWidth < fih2.biWidth) {
        BITMAPINFOHEADER fihTemp = fih2;
        fih2 = fih1;
        fih1 = fihTemp;

        BITMAPINFOHEADER fhTemp = fh2;
        fh2 = fh1;
        fh1 = fhTemp;

        char* dataTemp = data2;
        data2 = data1;
        data1 = dataTemp;
    }

    //Find real width bytes for the first
    int byteWidth = fih1.biWidth * 3;
    int padding = 4 - byteWidth % 4;
    if(padding == 4)
        padding = 0;
    int rwb1 = byteWidth + padding;

    LONG w1 = fih1.biWidth;
    LONG h1 = fih1.biHeight;

    //Find real width bytes for the second
    byteWidth = fih2.biWidth * 3;
    padding = 4 - byteWidth % 4;
    if(padding == 4)
        padding = 0;
    int rwb2 = byteWidth + padding;

    LONG w2 = fih2.biWidth;
    LONG h2 = fih2.biHeight;

    //Create result image memory
    char* newData = (char*)mmap(NULL,fih1.biSizeImage,PROT_READ|PROT_WRITE,MAP_SHARED|MAP_ANONYMOUS,-1,0);

    //Handle sizing
    float scaleX = (float)w2 / (float)w1;
    float scaleY = (float)h2 / (float)h1;


    //Process Image
    // for(int i = n - 1; i > 0; i--) {
    // imageProcess(0, 1, w1, h1, rwb1, rwb2, data1, data2, ratio, scaleX, scaleY, newData, w2, h2);
    // }

    imageProcess(0, n, w1, h1, rwb1, rwb2, data1, data2, ratio, scaleX, scaleY, newData, w2, h2);

    //Create bmp file
    FILE* lastFile = fopen(output, "wb");
    
    fwrite(&fh1.bfType, 2, 1, lastFile);
    fwrite(&fh1.bfSize, 4, 1, lastFile);
    fwrite(&fh1.bfReserved1, 2, 1, lastFile);
    fwrite(&fh1.bfReserved2, 2, 1, lastFile);
    fwrite(&fh1.bfOffBits, 4, 1, lastFile);

    fwrite(&fih1, sizeof(fih1), 1, lastFile);

    fwrite(newData, fih1.biSizeImage, 1, lastFile);

    fclose(lastFile);


    //Close up shop
    munmap(data1, fih1.biSizeImage);
    munmap(data2, fih2.biSizeImage);
    munmap(newData, fih1.biSizeImage); 
    return 0;
}