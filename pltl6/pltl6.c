#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#define BYTE unsigned char
void *startofheap = NULL;

/*
struct chunkinfo
{
    int size;               // size of the complete chunk including the chunkinfo
    int inuse;              // is it free? 0 or occupied? 1
    chunkinfo *next, *prev; // address of next and prev chunk
} typedef chunkinfo;
*/

typedef struct listelem
{
    char text[100];
    struct listelem *prev, *next;
} listelem;

/*
chunkinfo *get_last_chunk() // you can change it when you aim for performance
{
    if (!startofheap) // I have a global void *startofheap = NULL;
        return NULL;
    chunkinfo *ch = (chunkinfo *)startofheap;
    for (; ch->next; ch = (chunkinfo *)ch->next)
        ;
    return ch;
}
*/

int main()
{
    listelem *begin = (listelem *)malloc(10000);
    listelem *ptr = begin;
    listelem *head = ptr;
    listelem *tail = NULL;
    int count = 0;
    // Do something with the first

    while (1)
    {
        char *input[100];
        scanf("%100s", &input);

        if (strcmp(input, "deleteall") == 0)
        {
            ptr = head;
            ptr->next = NULL;

            strcpy(ptr->text, "");
            count = 0;
            tail = NULL;
        }
        else if (strcmp(input, "print") == 0)
        {
            if(count == 0) {
                printf("The list is empty.\n");
                continue;
            }

            listelem* cursor = head;
            while(cursor) {
                printf("%s\n", cursor->text);
                cursor = cursor->next;
            }
            
        }
        else if (strcmp(input, "insert") == 0)
        {
            scanf("%100s", &input);

            int x;
            scanf("%d", &x);
            if(x > count) {
                printf("Bad input, there are only %d\n", count);
                continue;

            }

            listelem* cursor = head;
            for(int i = 0; i < x; i++) {
                cursor = cursor->next;
            }

            strcpy(ptr->text, input);
            
            if(cursor == head) {
                head = ptr;
                ptr->next = cursor;
                ptr->prev = cursor->prev;
                cursor->prev = ptr;
            }
            else {
                (cursor->prev)->next = ptr;
                ptr->next = cursor;
                ptr->prev = cursor->prev;
                cursor->prev = ptr;
            }

            
            

            ptr += 1;
            count += 1;

        }
        else if (strcmp(input, "swap") == 0)
        {
            int x;
            scanf("%d", &x);

            if(x >= count - 1) {
                printf("Bad input, there are only %d\n", count);
                continue;

            }

            listelem* cursor = head;
            for(int i = 0; i < x; i++) {
                cursor = cursor->next;
            }

            listelem* temp = cursor;
            listelem* postcursor = cursor->next;

            if(cursor == head) {
                head = postcursor;
            }

            if(postcursor == tail) {
                tail = cursor;
            }

            cursor->next = postcursor->next;
            cursor->prev = postcursor;
            postcursor->next = temp;
            postcursor->prev = temp->prev;

        }
        else if (strcmp(input, "remove") == 0)
        {
            scanf("%100s", &input);

            listelem* cursor = head;
            while(strcmp(input, cursor->text) != 0) {
                if(cursor->next == NULL) {
                    cursor = NULL;
                    break;
                }
                else
                    cursor = cursor->next;
            }

            if(cursor == NULL) {
                continue;
            }

            //Check if it's the head
            if(cursor == head) {
                head = cursor->next;
                count--;
            }
            else if (cursor == tail) {
                listelem* precursor = cursor->prev;
                
                tail = precursor;
                tail->next = NULL;
                count--;
            }
            else { //There is a node before and after the cursor
                listelem* precursor = cursor->prev;
                listelem* postcursor = cursor->next;

                precursor->next = postcursor;
                postcursor->prev = precursor;

                count--;
            }

        }
        else if (strcmp(input, "quit") == 0)
        {
            break;
        }
        else
        {
            listelem *newNode = ptr;
            strcpy(newNode->text, input);

            if(tail == NULL) {
                tail = ptr;
                ptr->next = NULL;
                ptr->prev = NULL;
            }
            else {
                tail->next = ptr;
                ptr->prev = tail;
                ptr->next = NULL;
                tail = ptr;
            }
            
            //newNode->prev = ptr - 1;
            //(ptr - 1)->next = newNode;
            //ptr->next = NULL;

            ptr += 1;
            count += 1;
        }
    }

    free(begin);
}