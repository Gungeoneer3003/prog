#include <stdio.h>
#include <unistd.h>
#define BYTE unsigned char
void *startofheap = NULL;

struct chunkinfo
{
    int size;                      // size of the complete chunk including the chunkinfo
    int inuse;                     // is it free? 0 or occupied? 1
    struct chunkinfo *next, *prev; // address of next and prev chunk
} typedef chunkinfo;

chunkinfo *get_last_chunk() // you can change it when you aim for performance
{
    if (!startofheap) // I have a global void *startofheap = NULL;
        return NULL;
    chunkinfo *ch = (chunkinfo *)startofheap;
    for (; ch->next; ch = (chunkinfo *)ch->next)
        ;
    return ch;
}

// Based on the provided get_last_chunk() function
chunkinfo *smallestMatchingChunk(int size)
{
    if (!startofheap) // I have a global void *startofheap = NULL;
        return NULL;
    chunkinfo *ch = (chunkinfo *)startofheap;
    chunkinfo *smallest = NULL;

    for (; ch->next; ch = (chunkinfo *)ch->next)
    {
        if (ch->inuse == 0 && ch->size >= size)
        {
            if (smallest == NULL || ch->size <= smallest->size)
                smallest = ch;
        }
    }

    return smallest;
}

void analyze()
{
    printf("\n--------------------------------------------------------------\n");
    if (!startofheap)
    {
        printf("no heap");
        printf(", program break on address: %x\n", sbrk(0));
        return;
    }
    chunkinfo *ch = (chunkinfo *)startofheap;
    for (int no = 0; ch; ch = (chunkinfo *)ch->next, no++)
    {
        printf("%d | current addr: %x |", no, ch);
        printf("size: %d | ", ch->size);
        printf("info: %d | ", ch->inuse);
        printf("next: %x | ", ch->next);
        printf("prev: %x", ch->prev);
        printf(" \n");
    }
    printf("program break on address: %x\n", sbrk(0));
}

BYTE *mymalloc(int size)
{
    printf("");
    int pageCount = ((size + sizeof(chunkinfo)) / 4096) + 1;
    int properSize = pageCount * 4096;

    chunkinfo *ListPtr = startofheap;

    // Check if there's nothing in the heap
    if (startofheap == NULL)
    {
        ListPtr = sbrk(0);
        sbrk(sizeof(chunkinfo));

        ListPtr->inuse = 1;
        ListPtr->size = properSize;
        ListPtr->next = NULL;
        ListPtr->prev = NULL;

        BYTE *NewPtr = sbrk(properSize - sizeof(chunkinfo));
        startofheap = ListPtr;

        return NewPtr;
    }

    // Look for if there's a suitable and free chunk
    chunkinfo *PrevPtr = NULL;
    ListPtr = smallestMatchingChunk(size);

    // No such chunk
    if (ListPtr == NULL)
    {
        PrevPtr = get_last_chunk();
        ListPtr = sbrk(0);
        sbrk(sizeof(chunkinfo));

        ListPtr->inuse = 1;
        ListPtr->size = properSize;
        ListPtr->next = NULL;
        ListPtr->prev = PrevPtr;

        PrevPtr->next = ListPtr;

        BYTE *NewPtr = sbrk(properSize - sizeof(chunkinfo));

        return NewPtr;
    }

    // There's one available and it's the smallest

    // Split the chunk if necessary
    if (ListPtr->size > properSize)
    {
        ListPtr->size -= properSize;
        BYTE *move = (BYTE *)ListPtr;
        move += ListPtr->size; // This would still be a multiple of 4096

        chunkinfo *newPtr = (chunkinfo *)move;
        newPtr->size = properSize;
        newPtr->inuse = 1;
        newPtr->prev = ListPtr;
        newPtr->next = ListPtr->next;

        if (ListPtr->next != NULL)
            (ListPtr->next)->prev = newPtr;
        ListPtr->next = newPtr;
        
        newPtr++;
        BYTE *data = (BYTE *)newPtr;

        return data;
    }
    // No splitting needed
    else
    {
        ListPtr->inuse = 1;

        ListPtr++;
        BYTE *data = (BYTE *)ListPtr;

        return data;
    }
}

void myfree(BYTE *addr)
{
    if (addr == NULL)
        return;

    chunkinfo *ch = (chunkinfo *)addr - 1;
    ch->inuse = 0;

    chunkinfo *postCursor = ch->next;
    chunkinfo *preCursor = ch->prev;

    // Handle the next chunk
    if (postCursor && postCursor->inuse == 0)
    {
        ch->size += postCursor->size;
        ch->next = postCursor->next;

        if (postCursor->next != NULL)
            (postCursor->next)->prev = ch;
    }

    // Handle the previous chunk
    if (preCursor && preCursor->inuse == 0)
    {
        preCursor->size += ch->size;
        preCursor->next = ch->next;

        if (ch->next != NULL)
            (ch->next)->prev = preCursor;

        ch = preCursor;
    }

    postCursor = ch->next;
    preCursor = ch->prev;

    // Return memory if it's the last chunk
    if (postCursor == NULL)
    {
        brk(ch);

        if (preCursor == NULL)
            startofheap = NULL;
        else
            preCursor->next = NULL;
    }
}

void main()
{
    BYTE *a[100];
    analyze(); // 50% points
    for (int i = 0; i < 100; i++)
        a[i] = mymalloc(1000);
    for (int i = 0; i < 90; i++)
        myfree(a[i]);
    analyze(); // 50% of points if this is correct
    myfree(a[95]);
    a[95] = mymalloc(1000);
    analyze(); // 25% points, this new chunk should fill the smaller free one
    //(best fit)
    for (int i = 90; i < 100; i++)
        myfree(a[i]);
    analyze(); // 25% should be an empty heap now with the start address
    // from the program start
}