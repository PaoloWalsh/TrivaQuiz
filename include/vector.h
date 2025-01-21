#ifndef VECTOR_H
#define VECTOR_H

#include "constants.h"

typedef struct Vector {
    void** array;
    int size;
    int used;
} Vector;

//void createVector(Vector **); 
void initArray(Vector *); 
void insertArray(Vector *, void *);      
int findArray(Vector *, void *);        
void removeArray(Vector *, void *);
int getArrayLength(Vector *);
void freeArray(Vector *);                  
//void freeVector(Vector **);

#endif
