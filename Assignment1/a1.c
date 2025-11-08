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

void analyze()
{
    printf("\n--------------------------------------------------------------\n");
    if (!startofheap)
    {
        printf("no heap");
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
    int pageCount = ((size+sizeof(chunkinfo)) / 4096) + 1;
    int properSize = pageCount * 4096;

    chunkinfo *ListPtr = startofheap;
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
    else
    {
        chunkinfo *PrevPtr = NULL;

        ListPtr = smallestMatchingChunk(size);

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
        else
        {
            if (ListPtr->size > properSize) {
                ListPtr->size -= properSize;
                BYTE* move = (BYTE*) ListPtr;
                move += ListPtr->size; //This would still be a multiple of 4096

                chunkinfo* newPtr = (chunkinfo*) move;
                newPtr->size = properSize;
                newPtr->inuse = 1;
                newPtr->prev = ListPtr;
                newPtr->next = ListPtr->next;

                newPtr++;
                BYTE *data = (BYTE *)ListPtr;
                
                return data;
            }
            else {
                ListPtr->inuse = 1;

                ListPtr++;
                BYTE *data = (BYTE *)ListPtr;

                return data;
            }
        }
    }
}

void myfree(BYTE *addr)
{
    // Find the chunkinfo (the linked list)
    chunkinfo *cursor = (chunkinfo *)addr;
    cursor--;

    // Set inuse to 0
    cursor->inuse = 0;

    // Set up var's
    chunkinfo *precursor = cursor->prev;
    chunkinfo *postcursor = cursor->next;

    // Check if there's an empty one before
    if (precursor == NULL)
    {
        if (postcursor == NULL)
        {
            brk(cursor);
            startofheap = NULL;
            // brk, this is the only one
        }
        else
        {
            if (postcursor->inuse == 0)
            {
                // Pre doesn't exist, post does and is 0
                if (postcursor->next != 0)
                {
                    cursor->size = cursor->size + postcursor->size;
                    cursor->next = postcursor->next;
                    (postcursor->next)->prev = cursor;
                }
                // Combine the last two, as the next is inevitably in use
                else
                    brk(cursor); // sbrk, remove the last ones
            }
            else
            {
                // Next is in use, nothing to do
            }
        }
    }
    else
    {
        // Precursor isn't null
        if (precursor->inuse == 0)
        {
            if (postcursor == NULL)
            {
                brk(precursor);
                if(precursor == startofheap)
                    startofheap = NULL;
                // brk, it's only the first two
            }
            else
            {
                if (postcursor->inuse == 0)
                {
                    // both pre and post exist and are 0
                    if (postcursor->next != 0)
                    {
                        precursor->size = precursor->size + cursor->size + postcursor->size;
                        precursor->next = postcursor->next; // Combine all three, as the next is inevitably in use
                        (postcursor->next)->prev = precursor;
                    }
                    else {
                        brk(precursor); // brk, remove all three
                        if(precursor == startofheap)
                            startofheap = NULL;
                    }
                }
                else
                {
                    precursor->size = precursor->size + cursor->size;
                    precursor->next = postcursor;
                    postcursor->prev = precursor;
                    // Combine the first two
                }
            }
        }
        else
        {
            if (postcursor == NULL)
            {
                brk(precursor);
                if(precursor == startofheap)
                    startofheap = NULL;
                // brk, and remove the first
            }
            else
            {
                if (postcursor->inuse == 0)
                {
                    // both pre and post exist, post is 0
                    if (postcursor->next != 0)
                    {
                        cursor->size = cursor->size + postcursor->size;
                        cursor->next = postcursor->next;
                        (postcursor->next)->prev = cursor;
                    }
                    // Combine the last 2, as the next is inevitably in use
                    else
                        brk(cursor); // sbrk, remove the last 2
                }
                else
                {
                    // Do nothing
                }
            }
        }
    }
}

//Based on the provided get_last_chunk() function
chunkinfo *smallestMatchingChunk(int size) // you can change it when you aim for performance
{
    if (!startofheap) // I have a global void *startofheap = NULL;
        return NULL;
    chunkinfo *ch = (chunkinfo *)startofheap;
    chunkinfo *smallest = ch; 
    
    for (; ch->next; ch = (chunkinfo *)ch->next) {
        if(ch->inuse == 0 && ch->size >= size) {
            if(ch->size <= smallest->size) 
                smallest = ch;
        }
    }

    return smallest;
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