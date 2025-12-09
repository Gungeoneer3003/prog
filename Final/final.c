#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <unistd.h>
#include <sys/mman.h>

typedef unsigned char BYTE;
typedef unsigned short WORD;
typedef unsigned int DWORD;
typedef unsigned int LONG;
//Image file structs
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

struct {
    int start, end; //X-positions
    BYTE val;
    int line;
} typedef colorBatch;

//Prototypes
int compareColorBatch(const void *a, const void *b);

int compareColorBatch(const void *a, const void *b) {
    const colorBatch *ca = a;
    const colorBatch *cb = b;

    if (ca->start != cb->start)
        return (ca->start - cb->start);   // sort by start first

    return (ca->line - cb->line);         // then by line
}

int main(int argc, char** argv) {
    //Terminal Inputs (single input file)
    // if (argc < 2) {
    //     printf("Insufficient arguments, try again.\n");
    //     printf("Usage: ./decompress input.cwa output.bmp\n");
    //     return 0;
    // }
    // char* input = argv[1];
    // char* output = argv[2];
    
    //Hard-coded Inputs
    char *input = "compressed.eck"; 
    char *output = "finalImage.bmp";

    //Handle input
    FILE* f1 = fopen(input, "rb");
    if (f1 == NULL) {
        perror("Failed to open the file.");
        return 1;
    }

    //Take BMP Input1
    BITMAPFILEHEADER fh;
    BITMAPINFOHEADER fih;

    fread(&fh.bfType, 2, 1, f1);
    fread(&fh.bfSize, 4, 1, f1);
    fread(&fh.bfReserved1, 2, 1, f1);
    fread(&fh.bfReserved2, 2, 1, f1);
    fread(&fh.bfOffBits, 4, 1, f1);

    fread(&fih, sizeof(fih), 1, f1);

    BYTE* newData = (BYTE*)mmap(NULL, fih.biSizeImage, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);

    int batchCount[3];
    fread(&batchCount[2], sizeof(int), 1, f1); //red
    fread(&batchCount[1], sizeof(int), 1, f1); //green
    fread(&batchCount[0], sizeof(int), 1, f1);

    colorBatch* batchTable[3];
    for(int i = 0; i < 3; i++) {
        batchTable[i] = (colorBatch*)mmap(NULL, batchCount[i] * sizeof(colorBatch), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);

        for(int j = batchCount[i]-1; j >= 0; j--) {
            fread(&batchTable[i][j].start, sizeof(int), 1, f1);
            fread(&batchTable[i][j].end, sizeof(int), 1, f1);
            fread(&batchTable[i][j].val, sizeof(BYTE), 1, f1);
            fread(&batchTable[i][j].line, sizeof(int), 1, f1);

            qsort(batchTable[i], batchCount[i], sizeof(colorBatch), compareColorBatch);
        }
    }

    fclose(f1);

    int byteWidth = fih.biWidth * 3;
    int padding = 4 - byteWidth % 4;
    if (padding == 4) 
        padding = 0; 
    int rwb = byteWidth + padding;

    LONG w = fih.biWidth;
    LONG h = fih.biHeight;

    int batchCursor[] = {0, 0, 0};
    for (int y = 0; y < h; y++) {
        for (int x = 0; x < w; x++) {
            int c = x * 3 + y * rwb; // c for cursor
            
            for(int i = 0; i < 3; i++) {
                colorBatch current = batchTable[i][batchCursor[i]];

                if(current.line == y && current.start <= x && current.end > x)
                    newData[c + i] = current.val;
                else
                    batchCursor[i]++;
            }                
        }
    }

    //Create file
    FILE* lastFile = fopen(output, "wb");
    
    fwrite(&fh.bfType, 2, 1, lastFile);
    fwrite(&fh.bfSize, 4, 1, lastFile);
    fwrite(&fh.bfReserved1, 2, 1, lastFile);
    fwrite(&fh.bfReserved2, 2, 1, lastFile);
    fwrite(&fh.bfOffBits, 4, 1, lastFile);

    fwrite(&fih, sizeof(fih), 1, lastFile);

    fwrite(newData, fih.biSizeImage, 1, lastFile);
    fclose(lastFile);

    //Conclude Program
    for(int i = 0; i < 3; i++) {
        munmap(batchTable[i], batchCount[i] * sizeof(colorBatch));
    }
    munmap(newData, fih.biSizeImage);
    return 0;
}