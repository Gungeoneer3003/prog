#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

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

int main(int argc, char* argv[]) {
    //Handle Command Line Input
    if(argc != 5) {
        printf("Invalid input count. Try again.\n");
        return 0;
    }

    char* input = argv[1];
    char* output = argv[2];
    char* operation = argv[3];
    float factor = atof(argv[4]);
    
    printf("The inputs are as follows:\n");
    for(int i = 0; i < argc; i++)
        printf("\t%s\n", argv[i]);

    printf("Afterwards, the factor is %f\n", factor);


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

    char* data = (char*)malloc(fih.biSizeImage);
    fread(data, fih.biSizeImage, 1, f);

    fclose(f);


    //Find real width bytes
    int padding = 4 - fih.biWidth % 4;
    if(padding == 4)
        padding = 0;
    int rwb = (fih.biWidth * fih.biBitCount / 8) + padding;


    //Perform operations
    if (strcmp(operation, "contrast") == 0) {
        if(factor < 0 || factor > 100) {
            free(data);
            perror("Factor is out of range. It should be within [0,100].");
            return 2;
        }

        for(int x = 0; x < fih.biWidth; x++) {
            for(int y = 0; y < fih.biHeight; y++)  {
                int c = x*3 + y * rwb; //c for cursor

                float B = (unsigned char)(data[c]) / 255.0f;
                float G = (unsigned char)(data[c+1]) / 255.0f;
                float R = (unsigned char)(data[c+2]) / 255.0f;
                B = powf(B, factor);
                if (B < 0) B = 0;
                if (B > 1) B = 1;

                G = powf(G, factor);
                if (G < 0) G = 0;
                if (G > 1) G = 1;

                R = powf(R, factor);
                if (R < 0) R = 0;
                if (R > 1) R = 1;

                data[c] = (unsigned char)(B * 255.0f) ;
                data[c+1] = (unsigned char)(G* 255.0f);
                data[c+2] = (unsigned char)(R * 255.0f);

            }
        }
    }
    else if (strcmp(operation, "saturation") == 0) {
        if(factor < 0 || factor > 1) {
            free(data);
            perror("Factor is out of range. It should be within [0,1].");
            return 3;
        }
        
        for(int x = 0; x < fih.biWidth; x++) {
            for(int y = 0; y < fih.biHeight; y++)  {                
                int c = x*3 + y * rwb; //c for cursor

                float B = (unsigned char)(data[c]) / 255.0f;
                float G = (unsigned char)(data[c+1]) / 255.0f;
                float R = (unsigned char)(data[c+2]) / 255.0f;

                float avg = (B + G + R) / 3.0f;

                B = B + (B-avg) * factor;
                if (B < 0) B = 0;
                if (B > 1) B = 1;

                G = G + (G-avg) * factor;
                if (G < 0) G = 0;
                if (G > 1) G = 1;

                R = R + (R-avg) * factor;
                if (R < 0) R = 0;
                if (R > 1) R = 1;

                data[c] = (unsigned char)(B * 255.0f);
                data[c + 1] = (unsigned char)(G * 255.0f);
                data[c + 2] = (unsigned char)(R * 255.0f);
            }
        }
    }
    else if (strcmp(operation, "lightness") == 0) {
        if(factor < -1 || factor > 1) {
            free(data);
            perror("Factor is out of range. It should be within [-1,1].");
            return 3;
        }
        //factor = 0;

        for(int x = 0; x < fih.biWidth; x++) {
            for(int y = 0; y < fih.biHeight; y++)  {                
                int c = x*3 + y * rwb; //c for cursor

                float B = (unsigned char)(data[c]) / (255.0f) + factor;
                if (B < 0) B = 0;
                if (B > 1) B = 1;

                float G = (unsigned char)(data[c+1]) / (255.0f) + factor;
                if (G < 0) G = 0;
                if (G > 1) G = 1;

                float R = (unsigned char)(data[c+2]) / (255.0f) + factor;
                if (R < 0) R = 0;
                if (R > 1) R = 1;

                data[c] = (unsigned char)(B * 255.0f);
                data[c + 1] = (unsigned char)(G * 255.0f);
                data[c + 2] = (unsigned char)(R * 255.0f);
            }
        }
    }

    //Store results in output directory
    FILE* secondFile = fopen(output, "wb");
    
    fwrite(&fh.bfType, 2, 1, secondFile);
    fwrite(&fh.bfSize, 4, 1, secondFile);
    fwrite(&fh.bfReserved1, 2, 1, secondFile);
    fwrite(&fh.bfReserved2, 2, 1, secondFile);
    fwrite(&fh.bfOffBits, 4, 1, secondFile);

    fwrite(&fih, sizeof(fih), 1, secondFile);

    fwrite(data, fih.biSizeImage, 1, secondFile);

    fclose(secondFile);
    free(data);
    printf("Made it to the end.\n");
}