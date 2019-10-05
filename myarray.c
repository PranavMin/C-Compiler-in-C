// Author: Pranav Minasandram

#include "myarray.h"
#include "message.h"

//initializes array
void initArray(TA a, size_t initSize){
	a->array = (TT*) malloc(initSize * sizeof(TT));
	a->used = 0;
	a->size = initSize;
}

// adds element to tail of array
void insertArray(TA a, TT root){
	if(a->used == a->size){
		a->size *= 2;
		a->array = (TT *)realloc(a->array, a->size * sizeof(TT));
	}
	a->array[a->used++] = root;
}

// frees the array from memory
void freeArray(TA a){
	free(a->array);
    a->array = NULL;
	a->used = a->size = 0;
}

void createStack(WS stack, unsigned capacity){
    stack->capacity = capacity;
    stack->top = -1;
    stack->array = (char**) malloc(stack->capacity * sizeof(char*));
}
int isFull(WS stack){
    return stack->top == stack->capacity - 1;
}

int isEmpty(WS stack){
    return stack->top == -1;
}

void push(WS stack, char* item){
    if(isFull(stack))
        return;
    stack->array[++stack->top] = item;
}
char* pop(WS stack){
    if(isEmpty(stack)){
        error("break not inside switch or loop");
        return NULL;
    }
    return stack->array[stack->top--];
}

