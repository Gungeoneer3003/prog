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
int cmp_hte(const void *pa, const void *pb);
void pushNulls(hte **arr, int n);
void putbit(bitarr* x, BYTE bit);
int getbit(bitarr* x);
void putbitpattern(bitarr* x, bitpattern* source);
void writebit(bitpattern *d, BYTE bit);
void imageProcess(LONG, LONG, int, BYTE*, int, BYTE*, hte***);
int QualityToDivisor(int);
int sizeTree(hte* node);
void determinePath(hte* tree, bitpattern* table[], BYTE* path, int i);
void freeTree(hte **treeTable, int count);
void freeNode(hte *node);
void fwriteTree(hte* tree, FILE* f);
hte* freadTree(FILE* f);
BYTE determineByte(bitarr* arr, hte* tree);

int cmp_hte(const void *pa, const void *pb) {
    const hte *a = *(hte * const *)pa;
    const hte *b = *(hte * const *)pb; 

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

        x->data = (BYTE*)realloc(x->data, x->capacity * sizeof(BYTE));
        memset(x->data + x->size, 0, x->capacity - x->size);
    }
 
    x->data[byteIndex] |= (bit << (7 - bitIndex));
    
    if (byteIndex >= x->size)
        x->size = byteIndex + 1;
    x->bitp++;
}

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

void putbitpattern(bitarr* x, bitpattern* source) {
    for(int u = 0; u < source->digit; u++) {
        putbit(x, source->data[u]);
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

int sizeTree(hte* node) {
    if (node == NULL)
        return 0;
    return 1 + sizeTree(node->l) + sizeTree(node->r);
}

void determinePath(hte* tree, bitpattern* table[], BYTE* path, int i) {
    if(tree->l != NULL && tree->r != NULL) {
        path[i++] = 0;
        determinePath(tree->l, table, path, i);
        i--;

        path[i++] = 1;
        determinePath(tree->r, table, path, i);
        i--;
    }
    else if (tree->r != NULL) {
        path[i++] = 1;
        determinePath(tree->r, table, path, i);
        i--;
    }
    else if (tree->l != NULL) {
        path[i++] = 0;
        determinePath(tree->l, table, path, i);
        i--;
    }
    else {
        if (table[tree->val] == NULL) {
            table[tree->val] = malloc(sizeof(bitpattern));
            table[tree->val]->digit = 0;
        }

        int size = i;

        for(int i = 0; i < size; i++) {
            table[tree->val]->data[i] = path[i];
            table[tree->val]->digit++;
        }
    }
}

void freeTree(hte **table, int count) {
    for (int i = 0; i < count; i++) {
        if (table[i]) {
            freeNode(table[i]);    // frees malloc'ed tree nodes
        }
    }
    munmap(table, sizeof(hte*) * count);  // free the mmap'ed pointer array
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

    // Write only the data we actually need
    fwrite(&tree->frq, sizeof(tree->frq), 1, f);
    fwrite(&tree->val, sizeof(tree->val), 1, f);

    fwriteTree(tree->l, f);
    fwriteTree(tree->r, f);
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

//This assumes no bad paths
BYTE determineByte(bitarr* arr, hte* tree) {
    if(tree->l == NULL && tree->r == NULL) 
        return tree->val;
    
    if(getbit(arr) == 0)
        return determineByte(arr, tree->l);
    else
        return determineByte(arr, tree->r);
}



//Table of contents of main:
/*
1. Read in inputs
2. Read in file
3. Create supporting image file variables
4. Allocate hte array for the tree
5. Process Image
6. Determine size of each tree and push nulls to the bottom
7. Create each tree, then determine paths for each val
8. Create bitarrs, and fill each one with the new compressed data
9. Create a file to store the compressed image
10. Store supporting image file variables
11. Store each tree, each bitarr, and relevant info for each
12. Close the file
13. Deallocate all used memory (arrays, trees, and bitarrs)
*/

int main(int argc, char** argv) {
    //Terminal Inputs (single input file)
    // if (argc < 2) {
    //     printf("Insufficient arguments, try again.\n");
    //     printf("Usage: ./a2 input.bmp ratio processors output.bmp\n");
    //     return 0;
    // }
    // char* input = argv[1];
    // int quality = atoi(argv[2]);

    //Hard coded inputs
    char *input = "lion.bmp";
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

    BYTE* data = (BYTE*)mmap(NULL, fih.biSizeImage, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    fread(data, fih.biSizeImage, 1, f1);

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

    //Set up other variables
    int d = QualityToDivisor(quality);
    int size = 255 / d; //This represents the max value a pixel can be in the table
    
    
    hte **treeTable[3]; //Some space is unused, this is simpler
    for(int i = 0; i < 3; i++) {
        treeTable[i] = (hte**)mmap(NULL, sizeof(hte*) * (size + 1), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
        for(int j = 0; j < size + 1; j++)
            treeTable[i][j] = NULL;
    }

    imageProcess(w, h, rwb, data, d, newData, treeTable);
    
    int tableSize[] = {0, 0, 0}; //BGR
    for(int i = 0; i < 3; i++) {
        for(int j = 0; j < size + 1; j++) {
            hte *p = treeTable[i][j];
            if(p != NULL)
                tableSize[i]++;
        }

        pushNulls(treeTable[i], size + 1); //Not very efficient, but it pushes them after counting
    }     

    //Make the trees and figure out size
    int treeSize[] = {0, 0, 0};
    bitpattern** codeTable[3];
    for(int i = 0; i < 3; i++) {
        int val = tableSize[i];

        while(treeTable[i][1] != NULL) { //Loop breaks after there's only 1 left
            qsort(treeTable[i], tableSize[i], sizeof(hte*), cmp_hte);

            hte *comb = (hte*)malloc(sizeof(hte));
            comb->l = treeTable[i][0];
            comb->r = treeTable[i][1];
            comb->frq = treeTable[i][0]->frq + treeTable[i][1]->frq;
            comb->val = -1;

            treeTable[i][0] = comb;
            treeTable[i][1] = NULL;

            pushNulls(treeTable[i], tableSize[i]);
            tableSize[i]--;
        } 

        tableSize[i] = val; //Restore it for later
        treeSize[i] = sizeTree(treeTable[i][0]);

        //Now that the tree is done, make the path table
        codeTable[i] = (bitpattern**)mmap(NULL, sizeof(bitpattern*) * (size + 1), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
        for (int v = 0; v <= size; v++) {
            codeTable[i][v] = NULL;
        }
        
        BYTE path[200];
        determinePath(treeTable[i][0], codeTable[i], path, 0);
    }

    //Create bitarrs and fill them
    bitpattern* tempPath;
    bitarr* arr[3];
    for(int u = 0; u < 3; u++) { //u for current color
        arr[u] = (bitarr*)mmap(NULL, sizeof(bitarr), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
        arr[u]->data = NULL;
        arr[u]->bitp = 0;
        arr[u]->size = 0;
        arr[u]->capacity = 0;


        for (int x = 0; x < w; x++)
        {
            for (int y = 0; y < h; y++)
            {
                int c = x * 3 + y * rwb; // c for cursor
                int val = newData[c + u]; 
                
                tempPath = codeTable[u][val];
                putbitpattern(arr[u], tempPath);
            }
        }
    }

    //File Storage
    /*
    1. All normal image variables
    2. Size of a code table, then the table
    3. Size of a bitarr, then the bitarr for one color
    4. (Part 2 and 3 for the other two colors)
    */

    //Return the modified image with a different title
    size_t len = strlen(input);

    char output[256];      // or dynamically allocate
    memcpy(output, input, len - 4);
    output[len - 4] = '\0';

    FILE* lastFile = fopen(output, "wb");
    
    //Part 1
    fwrite(&fh.bfType, 2, 1, lastFile);
    fwrite(&fh.bfSize, 4, 1, lastFile);
    fwrite(&fh.bfReserved1, 2, 1, lastFile);
    fwrite(&fh.bfReserved2, 2, 1, lastFile);
    fwrite(&fh.bfOffBits, 4, 1, lastFile);

    fwrite(&fih, sizeof(fih), 1, lastFile);

    fwrite(&quality, sizeof(int), 1, lastFile);
    //Part 2-4
    for(int i = 0; i < 3; i++) {
        //Write the tree in 
        fwriteTree(treeTable[i][0], lastFile); //Recursive function
        
        fwrite(&arr[i]->size, sizeof(arr[i]->size), 1, lastFile);
        fwrite(arr[i]->data, arr[i]->size, 1, lastFile);
        fwrite(&arr[i]->bitp, sizeof(arr[i]->bitp), 1, lastFile);
        fwrite(&arr[i]->capacity, sizeof(arr[i]->capacity), 1, lastFile);
    }
    fclose(lastFile);


    //Conclude Program
    for(int i = 0; i < 3; i++) {
        for(int j = 0; j <= size; j++) {
            if(codeTable[i][j] != NULL)
                free(codeTable[i][j]);
        }
        munmap(codeTable[i], sizeof(bitpattern*) * (size + 1));
        
        freeTree(treeTable[i], size + 1);

        free(arr[i]->data);
        munmap(arr[i], sizeof(bitarr));
    }
    munmap(data, fih.biSizeImage);
    munmap(newData, fih.biSizeImage);
    return 0;
}