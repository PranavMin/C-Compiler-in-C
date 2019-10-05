#ifndef T_ARR
#define T_ARR
#include <limits.h>
#include "tree.h"
#include <stdlib.h>
// dynamically sizing array for storing declarators
typedef struct ta{
	TT *array;
	size_t used;
	size_t size;
} TreeArray, *TA;

void initArray(TA a, size_t initSize);
void insertArray(TA a, TT root);
void freeArray(TA a);

typedef struct ws {
    int top;
    unsigned capacity;
    char** array;
} WHILE_STACK, *WS;

void createStack(WS stack, unsigned capacity);
int isFull(WS stack);
int isEmpty(WS stack);
void push(WS stack, char* item);
char* pop(WS stack);


#endif
