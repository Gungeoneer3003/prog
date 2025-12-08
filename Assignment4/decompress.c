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
int cmp_hte(hte*, hte*);
void pushNulls(hte **arr, int n);
void putbit(bitarr* x, BYTE bit);
int getbit(bitarr* x);
void putbitpattern(bitarr* x, bitpattern source);
void writebit(bitpattern *d, BYTE bit);
void imageProcess(LONG, LONG, int, BYTE*, int, BYTE*, hte***);
int QualityToDivisor(int);
int sizeTree(hte* node);
void determinePath(hte* tree, bitpattern* table, BYTE* path, int i);
void freeTree(hte *treeTable);
void freeNode(hte *node);
void fwriteTree(hte* tree, FILE* f);
hte* freadTree(FILE* f);
BYTE determineByte(bitarr* arr, hte* tree);

int cmp_hte(hte *a, hte *b) {
    int aFrq = a->frq;
    int bFrq = b->frq;
    return (aFrq > bFrq) - (aFrq < bFrq); 
}

void pushNulls(hte **arr, int n) {
    int write = 0;  // where the next non-null should go

    for (int read = 0; read < n; read++) {
        if (arr[read] != NULL) {
            arr[write++] = arr[read];
        }
    }

    // fill the rest with NULL
    while (write < n) {
        arr[write++] = NULL;
    }
}

void putbit(bitarr *x, BYTE bit) {
    int byteIndex = x->bitp / 8;
    int bitIndex = x->bitp % 8;

    //Need code for resizing
    if (x->data == NULL) {
        x->capacity = 4;
        x->data = (BYTE*)calloc(x->capacity, 1);
        x->size = 0;
    } 
    else if (byteIndex >= x->capacity) {
        while(byteIndex >= x->capacity)
            x->capacity *= 2; //Double it

        x->data = (BYTE*)realloc(x->data, x->capacity * sizeof(int));
        memset(x->data + x->size, 0, x->capacity - x->size);
    }
 
    x->data[byteIndex] |= (bit << (7 - bitIndex));
    
    if (byteIndex >= x->size)
        x->size = byteIndex + 1;
    x->bitp++;
}

int getbit(bitarr* x) {
    int i = x->bitp / 8;
    int actual_bitp = x.bitp - i * 8;

    BYTE bit = 1;
    int shiftamount = 8 - 1 - actual_bitp;
    bit <<= shiftamount;

    BYTE targetbit = x.data[i] & bit;
    x->bitp++;

    if (targetbit == 0) return 0;
    else return 1;
}

void putbitpattern(bitarr* x, bitpattern source) {
    for(int u = 0; u < source.digit; u++) {
        putbit(x, source.data[u]);
    }
}

void writebit(bitpattern *d, BYTE bit) {
    d->data[d->digit++] = bit;
}

void imageProcess(LONG w, LONG h, int rwb, BYTE* data, int divisor, BYTE* newData, hte** treeTable[]) {
    for (int x = 0; x < w; x++)
    {
        for (int y = 0; y < h; y++)
        {
            int c = x * 3 + y * rwb; // c for cursor

            int bgr[] = {-1, -1, -1};
            for(int i = 0; i < 3; i++)
                bgr[i] = data[c + i] / divisor;

            //Store data in a table that has three arrays from [0, 255 / divisor]
            for(int i = 0; i < 3; i++) {
                int index = bgr[i];
                hte *p = treeTable[i][index];

                if (p == NULL) {
                    treeTable[i][index] = (hte*)malloc(sizeof(hte));
                    p = treeTable[i][index];

                    p->frq = 0;
                    p->val = index;
                    p->l = NULL;
                    p->r = NULL;
                }

                p->frq++;
            }


            newData[c] = bgr[0];
            newData[c + 1] = bgr[1];
            newData[c + 2] = bgr[2];
        }
    }
}

int QualityToDivisor(int Quality) {
    int d;

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

int sizeTree(hte* node) {
    if (node == NULL)
        return 0;
    return 1 + sizeTree(node->l) + sizeTree(node->r);
}

void determinePath(hte* tree, bitpattern* table, BYTE* path, int i) {
    if(tree->l != NULL && tree->r != NULL) {
        path[i++] = 0;
        determinePath(tree->l, table, path, i);
        i--;
    }
    else if (tree->r != NULL) {
        path[i++] = 1;
        determinePath(tree->r, table, path, i);
        i--;
    }
    else {
        table[tree->val].digit = 0;
        int size = i;

        for(int i = 0; i < size; i++) {
            table[tree->val].data[i] = path[i];
            table[tree->val].digit++;
        }
    }
}

void freeTree(hte *treeTable) {
    for(int i = 0; i < 256; i++) {
        hte *node = treeTable[i];
        freeNode(node);
    }
    munmap(treeTable, sizeof(hte) * 256);
}

void freeNode(hte *node) {
    if (node == NULL)
        return;
    freeNode(node->l);
    freeNode(node->r);
    free(node);
}

void fwriteTree(hte* tree, FILE* f) {
    if (tree == NULL) {
        int flag = 0;
        fwrite(&flag, sizeof(int), 1, f);
        return;
    }
    int flag = 1;
    fwrite(&flag, sizeof(int), 1, f);
    fwrite(tree, sizeof(hte) - 2*sizeof(hte*), 1, f); // write data only
    fwriteTree(tree->l, f);
    fwriteTree(tree->r, f);
}

hte* freadTree(FILE* f) {
    int flag;
    fread(&flag, sizeof(int), 1, f);
    if (flag == 0)
        return NULL;

    hte* node = malloc(sizeof(hte));
    fread(node, sizeof(hte) - 2*sizeof(hte*), 1, f);

    node->l = freadTree(f);
    node->r = freadTree(f);
    return node;
}

//This assumes no bad paths
BYTE determineByte(bitarr* arr, hte* tree) {
    if(tree->l == NULL && tree->r == NULL) 
        return tree->val;
    
    if(getbit(arr) == 0)
        return determineByte(arr, tree->l);
    else
        return determineByte(arr, tree->r);
}


int main(int argc, char** argv) {
    //Terminal Inputs (single input file)
    // if (argc < 2) {
    //     printf("Insufficient arguments, try again.\n");
    //     printf("Usage: ./a2 input.bmp ratio processors output.bmp\n");
    //     return 0;
    // }
    // char* input = argv[1];
    // int quality = atoi(argv[2]);

    //Input file should be exactly as described
    //Hard coded inputs
    char *input = "lion"; 
    int quality = 10;

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
    int d = QualityToDivisor(quality);
    int size = 255 / d; //This represents the max value a pixel can be in the table


    int treeSize[3];
    hte* treeTable[3];
    bitarr arr[3];
    for(int i = 0; i < 3; i++) {
        fread(&treeSize[i], sizeof(treeSize[i]), 1, f1); 
        treeTable[i] = freadTree(f1);

        fread(arr[i].size, sizeof(arr[i].size), 1, f1);
        arr[i].data = (BYTE*)mmap(NULL, arr[i].size, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
        fread(arr[i].data, arr[i].size, 1, f1);
        fread(arr[i].bitp, sizeof(arr[i].bitp), 1, f1);
        fread(arr[i].capacity, sizeof(arr[i].capacity), 1, f1);

        arr[i].bitp = 0;
    }

    fclose(f1);

    //Find real width bytes for the first
    int byteWidth = fih.biWidth * 3;
    int padding = 4 - byteWidth % 4;
    if (padding == 4) 
        padding = 0; 
    int rwb = byteWidth + padding;

    LONG w = fih.biWidth;
    LONG h = fih.biHeight;

    //Create result image memory
    BYTE* newData = (BYTE*)mmap(NULL, fih.biSizeImage, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    
    
    //Process Image here
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
    char* output = "lion2.bmp";
    FILE* lastFile = fopen(output, "wb");
    
    //Part 1
    fwrite(&fh.bfType, 2, 1, lastFile);
    fwrite(&fh.bfSize, 4, 1, lastFile);
    fwrite(&fh.bfReserved1, 2, 1, lastFile);
    fwrite(&fh.bfReserved2, 2, 1, lastFile);
    fwrite(&fh.bfOffBits, 4, 1, lastFile);

    fwrite(&fih, sizeof(fih), 1, lastFile);

    fwrite(newData, fih.biSizeImage, 1, lastFile);
    fclose(lastFile);

    //Conclude Program
    munmap(newData, fih.biSizeImage);
    return 0;
}

