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

struct hte {
    int frq, val;
    struct hte *l, *r;
} typedef hte;

typedef struct {
    BYTE* data;
    int bitp, size, capacity;
} bitarr;

void imageProcess(LONG, LONG, int, BYTE*, int, BYTE*, hte***);
int f(int);
int cmp_hte(hte*, hte*);

// Initialize a dynamic array
void initDynamicArray(DynamicArray *arr, size_t initialCapacity) {
    arr->data = (int*)malloc(initialCapacity * sizeof(int));
    if (arr->data == NULL) {
        perror("Failed to allocate memory");
        exit(EXIT_FAILURE);
    }
    arr->size = 0;
    arr->capacity = initialCapacity;
}

// Add an element to the dynamic array
void addElement(DynamicArray *arr, int value) {
    if (arr->size == arr->capacity) {
        arr->capacity *= 2; // Double the capacity
        arr->data = (int *)realloc(arr->data, arr->capacity * sizeof(int));
        if (arr->data == NULL) {
            perror("Failed to reallocate memory");
            exit(EXIT_FAILURE);
        }
    }

    arr->data[arr->size++] = value;
}

// Free the memory allocated for the dynamic array
void freeDynamicArray(DynamicArray *arr) {
    free(arr->data);
    arr->data = NULL;
    arr->size = 0;
    arr->capacity = 0;
}

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
    int i = x->bitp / 8;
    int actual_bitp = bitp - i * 8;

    int shiftamount = 8 - 1 - actual_bitp;
    bit <<= shiftamount;

    if(x->data == NULL) {
        int initialCapacity = 4;

        x->data = (int*)malloc(initialCapacity * sizeof(int));
        x->size = 0;
        x->capacity = initialCapacity;
    }
    else if (x->size == x->capacity) {
        x->capacity *= 2; //Double it
        x->data = (int*)realloc(x->data, x->capacity * sizeof(int));
    }

    x->data[i] =| bit;
    x->size++;
    x->bitp++;
}


void putbitpattern(BYTE* bits, int bitsize) {
    for(int u = 0; u < bitsize / 8; u++) {
        int mom_bits = bitsize - u * 8;
        if (mom_bits > 8)
            mom_bits = 8;

        for(int i = 0; i < 8 && i < mom_bits; i++) {
            BYTE workon = bits[u];

            workon <<= i;
            workon >>= 7;

            putbit(workon);
        }
    }
}


void imageProcess(LONG w, LONG h, int rwb, BYTE* data, int divisor, BYTE* newData, hte*** table) {
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
                hte *p = table[i][index];

                if (p == NULL) {
                    table[i][index] = (hte*)malloc(sizeof(hte));
                    p = table[i][index];

                    p->frq = 0;
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

int f(int Quality) {
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
    int d = f(quality);
    int size = 255 / d; //This represents the max value a pixel can be in the table
    
    
    hte **table[3]; //Some space is unused, this is simpler
    for(int i = 0; i < 3; i++) {
        table[i] = (hte**)mmap(NULL, sizeof(hte) * (size + 1), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
        for(int j = 0; j < size + 1; j++)
            table[i][j] = NULL;
    }

    imageProcess(w, h, rwb, data, d, newData, table);
    
    int tableSize[] = {0, 0, 0}; //BGR
    for(int i = 0; i < 3; i++) {
        for(int j = 0; j < size + 1; j++) {
            hte *p = table[i][j];
            if(p != NULL)
                tableSize[i]++;
        }

        pushNulls(table[i], size + 1); //Not very efficient, but it pushes them after counting
    }

    //Make the tree
    for(int i = 0; i < 3; i++) {
        while(table[i][1] != NULL) { //Loop breaks after there's only 1 left
            qsort(table[i], tableSize[i], sizeof(hte*), cmp_hte);

            hte *comb = (hte*)malloc(sizeof(hte));
            comb->l = table[i][0];
            comb->r = table[i][1];
            comb->frq = table[i][0]->frq + table[i][1]->frq;
            comb->val = -1;

            table[i][0] = comb;
            table[i][1] = NULL;

            pushNulls(table[i], tableSize[i]);
            tableSize[i]--;
        } 
 
    }

    //Return the modified image with a different title
    char output[sizeof(input) / sizeof(char)];
    strncpy(output, input, sizeof(input) - 4);
    output[sizeof(output) / sizeof(char) - 1] = '\0';


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
    for(int i = 0; i < 3; i++)
        munmap(table[i], sizeof(hte) * size);
    munmap(data, fih.biSizeImage);
    munmap(newData, fih.biSizeImage);
    return 0;
}