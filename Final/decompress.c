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
void processImage(int i, LONG w, LONG h, int rwb, colorBatch** validBatchTable, int* valid, BYTE* newData);

void processImage(int i, LONG w, LONG h, int rwb, colorBatch** validBatchTable, int* valid, BYTE* newData) {
    int batchCursor = 0;

    for (int y = 0; y < h; y++) {
        for (int x = 0; x < w; x++) {
            int c = x * 3 + y * rwb; // c for cursor
            
           
                while (batchCursor < valid[i]) {
                    colorBatch current = validBatchTable[i][batchCursor];

                    // Batch is fully before current scan position: advance
                    if (current.line < y || (current.line == y && current.end <= x && current.start <= x)) {
                        batchCursor++;
                        continue;
                    }


                    // Batch is for a future row or starts after current x: stop here,
                    // nothing to cover this pixel for this color
                    if (current.line > y || current.start > x) {
                        break;
                    }

                    // Now we know: current.line == y && current.start <= x < current.end
                    newData[c + i] = current.val;
                    break;
                }
                            
        }
    }
}

int compareColorBatch(const void *a, const void *b) {
    const colorBatch *ca = a;
    const colorBatch *cb = b;

    if (ca->line < cb->line) return -1;
    if (ca->line > cb->line) return 1;

    if (ca->start < cb->start) return -1;
    if (ca->start > cb->start) return 1;

    return 0;
}

int main(int argc, char** argv) {
    //Terminal Inputs (single input file)
    if (argc < 2) {
        printf("Insufficient arguments, try again.\n");
        printf("Usage: ./decompress input.cwa output.bmp\n");
        return 0;
    }
    char* input = argv[1];
    char* output = argv[2];
    
    //Hard-coded Inputs
    // char *input = "compressed.eck"; 
    // char *output = "finalImage.bmp";

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

    int byteWidth = fih.biWidth * 3;
    int padding = 4 - byteWidth % 4;
    if (padding == 4) 
        padding = 0; 
    int rwb = byteWidth + padding;

    LONG w = fih.biWidth;
    LONG h = fih.biHeight;

    BYTE* newData = (BYTE*)mmap(NULL, fih.biSizeImage, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    memset(newData, 0, fih.biSizeImage);

    int batchCount[3];
    fread(&batchCount[2], sizeof(int), 1, f1); //red
    fread(&batchCount[1], sizeof(int), 1, f1); //green
    fread(&batchCount[0], sizeof(int), 1, f1);

    colorBatch* batchTable[3];
    colorBatch* validBatchTable[3]; //There's a lot of garbage in the file
    int valid[] = {0, 0, 0};

    for(int i = 3-1; i >= 0; i--) { //Red, then green, then blue
        batchTable[i] = (colorBatch*)mmap(NULL, batchCount[i] * sizeof(colorBatch), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
        validBatchTable[i] = (colorBatch*)mmap(NULL, batchCount[i] * sizeof(colorBatch), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);

        for(int j = 0; j < batchCount[i]; j++) {
            fread(&batchTable[i][j], sizeof(colorBatch), 1, f1);

            if(batchTable[i][j].line < 0 || batchTable[i][j].line >= h)
                continue;
            
            if(batchTable[i][j].start < 0 || batchTable[i][j].start >= w)
                continue;
            
            if(batchTable[i][j].end < 0 || batchTable[i][j].end >= w)
                continue;
            
            if(batchTable[i][j].start > batchTable[i][j].end)
                continue;

            validBatchTable[i][valid[i]++] = batchTable[i][j];
        }

        qsort(validBatchTable[i], valid[i], sizeof(colorBatch), compareColorBatch);
    }

    fclose(f1);

    struct timeval tv1, tv2;
    gettimeofday(&tv1, NULL);

    int i = 2;
    while(i > 0) {
        int pid = fork();
        if (pid == 0) {
            processImage(i, w, h, rwb, validBatchTable, valid, newData);
            return 0;
        }
        i--;
    }

    processImage(i, w, h, rwb, validBatchTable, valid, newData);

    while(wait(0) != -1); 
    printf("All processes complete.\n");
    gettimeofday(&tv2, NULL);
    long long timing = (tv2.tv_sec - tv1.tv_sec) * 1000000LL + (tv2.tv_usec - tv1.tv_usec);
    printf("The operation took %lld microseconds.\n", timing);

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