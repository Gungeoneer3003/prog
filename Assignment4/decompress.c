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

//Supporting structs
struct hte {
    int frq, val;
    struct hte *l, *r;
} typedef hte;

typedef struct {
    int size;
    BYTE* data;
    int bitp, capacity;
} bitarr;

typedef struct {
    BYTE data[200]; //Currently unknown if a smaller digit is more practical
    int digit; 
} bitpattern;

//Prototypes
int getbit(bitarr* x);
BYTE determineByte(bitarr* arr, hte* tree);
int QualityToDivisor(int);
hte* freadTree(FILE* f);
void freeNode(hte *node);

//Functions
int getbit(bitarr* x) {
    int i = x->bitp / 8;
    int actual_bitp = x->bitp - i * 8;

    BYTE bit = 1;
    int shiftamount = 8 - 1 - actual_bitp;
    bit <<= shiftamount;

    BYTE targetbit = x->data[i] & bit;
    x->bitp++;

    if (targetbit == 0) return 0;
    else return 1;
}

BYTE determineByte(bitarr* arr, hte* tree) {
    if(tree->l == NULL && tree->r == NULL) 
        return tree->val;
    
    if(getbit(arr) == 0)
        return determineByte(arr, tree->l);
    else
        return determineByte(arr, tree->r);
}

int QualityToDivisor(int Quality) {
    int d = 1;

    switch (Quality) {
        case 1:
            d = 25;
            break;
        case 2:
            d = 22;
            break;
        case 3:
            d = 17;
            break;
        case 4:
            d = 13;
            break;
        case 5:
            d = 10;
            break;
        case 6:
            d = 7;
            break;
        case 7:
            d = 5;
            break;
        case 8:
            d = 4;
            break;
        case 9:
            d = 2;
            break;
        case 10:
            d = 1;
            break;
        default:
            printf("Invalid quality value. Please enter a number from 1 to 10.\n");
    }

    return d;
}

hte* freadTree(FILE* f) {
    int flag;
    if (fread(&flag, sizeof(int), 1, f) != 1)
        return NULL;  // handle EOF / error

    if (flag == 0)
        return NULL;

    hte* node = malloc(sizeof(hte));
    if (!node) return NULL;

    fread(&node->frq, sizeof(node->frq), 1, f);
    fread(&node->val, sizeof(node->val), 1, f);

    node->l = freadTree(f);
    node->r = freadTree(f);
    return node;
}

void freeNode(hte *node) {
    if (node == NULL)
        return;
    freeNode(node->l);
    freeNode(node->r);
    free(node);
}


int main(int argc, char** argv) {
    //Terminal Inputs (single input file)
    // if (argc < 2) {
    //     printf("Insufficient arguments, try again.\n");
    //     printf("Usage: ./decompress input.cwa output.bmp\n");
    //     return 0;
    // }
    char* input = argv[1];
    
    //Hard-coded Inputs
    char *input = "lion.cwa"; 

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

    //Set up other variables
    int quality;
    fread(&quality, sizeof(int), 1, f1);
    int d = QualityToDivisor(quality);

    hte* treeTable[3]; //Some space is unused, this is simpler
    bitarr* arr[3];
    for(int i = 0; i < 3; i++) {
        treeTable[i] = freadTree(f1);

        arr[i] = (bitarr*)malloc(sizeof(bitarr));

        fread(&arr[i]->size, sizeof(arr[i]->size), 1, f1); 
        arr[i]->data = (BYTE*)malloc(arr[i]->size);
        
        fread(arr[i]->data, arr[i]->size, 1, f1);
        fread(&arr[i]->bitp, sizeof(arr[i]->bitp), 1, f1);
        arr[i]->bitp = 0;
        fread(&arr[i]->capacity, sizeof(arr[i]->capacity), 1, f1);
    }
    
    fclose(f1);

    int byteWidth = fih.biWidth * 3;
    int padding = 4 - byteWidth % 4;
    if (padding == 4) 
        padding = 0; 
    int rwb = byteWidth + padding;

    LONG w = fih.biWidth;
    LONG h = fih.biHeight;

    //Create result image memory
    BYTE* newData = (BYTE*)mmap(NULL, fih.biSizeImage, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    
    //Fill each pixel
    for (int x = 0; x < w; x++)
    {
        for (int y = 0; y < h; y++)
        {
            int c = x * 3 + y * rwb; // c for cursor
            
            newData[c] = determineByte(arr[0], treeTable[0]) * d;
            newData[c + 1] = determineByte(arr[1], treeTable[1]) * d;
            newData[c + 2] = determineByte(arr[2], treeTable[2]) * d;
        }
    }

    //Create file
    char* output = argv[1];
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
        freeNode(treeTable[i]);
        free(arr[i]->data);
        free(arr[i]);
    }
    munmap(newData, fih.biSizeImage);
    return 0;
}