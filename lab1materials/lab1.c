#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>  //for tolower()
#include <string.h> //for strcpy()

struct listelement
{ 
    struct listelement *next, *prev;
    char text[1000];
};
typedef struct listelement node;


struct alphabetElement {
    char L;
    node* words;    //To hold the list for that alphabet
    struct alphabetElement *next, *prev;
};
typedef struct alphabetElement listNode; 

//Prototypes
void insert(char* txt, listNode* headPtr);
void outerPrint(listNode* headPtr);
void innerPrint(node* headPtr);
void LLDelete(node* headPtr);
listNode* makeAlphabet();
void deleteAlphabet(listNode* headPtr);


void insert(char* txt, listNode* headPtr) {
    char first = tolower(txt[0]);

    for(listNode* c = headPtr; c != 0; c = c->next) {
        if(c->L == first) {
            node* newNode = (node*)malloc(sizeof(node));
            strcpy(newNode->text, txt); //This assumes that txt is 1000 characters and null terminated
            
            if(c->words) {
                node* innerC;
                for(innerC = (c->words); innerC->next != 0; innerC = innerC->next);
                
                innerC->next = newNode;
                newNode->prev = innerC;
            }
            else 
                c->words = newNode;
            return;
        }
    } 

    printf("The first letter isn't from the alphabet. Try again.\n");
}

void innerPrint(node* headPtr) {
    for(node* c = headPtr; c != 0; c = c->next) {
        printf("%s\n", c->text);
    } 
}

void outerPrint(listNode* headPtr) {
    for(listNode* c = headPtr; c != 0; c = c->next) {
        if(c->words)
            innerPrint(c->words);
    }
}

void LLDelete(node* headPtr) {
    node *cursor;
    node *postcursor = NULL;
    
    cursor = headPtr;
    while (cursor) {
        postcursor = cursor->next;
        free(cursor);
        cursor = postcursor;
    }

}

//Make the list containing the full alphabet
listNode* makeAlphabet() {
    listNode* list = NULL;
    listNode* ptr;
    listNode* preptr = NULL;

    //Make a loop for the characters 
    for(char letter = 'a'; letter <= 'z'; letter++) {
        ptr = (listNode*)malloc(sizeof(listNode));
        
        ptr->L = letter; //Letter isn't important, but helps tracking and debugging
        ptr->words = NULL;
        ptr->prev = preptr;
        
        if(list == NULL) 
            list = ptr;
        else
            preptr->next =  ptr;

        preptr = ptr;
    }

    return list;
}

//Similar to LLDelete, but go through each in the Alphabet linked list
void deleteAlphabet(listNode* headPtr) {
    listNode *cursor = headPtr;
    listNode *postcursor = NULL;
    
    cursor = headPtr;
    if (cursor)
        postcursor = cursor->next;
        
    while (cursor) {
        LLDelete(cursor->words);
        postcursor = cursor->next;
        free(cursor);
        cursor = postcursor;
    }
}

int main() {
    node* head = NULL;
    head = (node*)malloc(sizeof(node));

    printf("Made it here!\n");
    listNode* list = makeAlphabet();
    char input[1000];
    
    while (1) {
        scanf("%999s", input);
        if(strcmp(input, "print") != 0)
            insert(input, list);
        else
            break;
    }


    printf("\nYour list is: \n");
    outerPrint(list);
    
    deleteAlphabet(list);
    free(head);
    return 0;
}

