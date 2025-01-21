#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "../include/vector.h"

// alloca l'array di void* sullo heap e inizializza le variabili
void initArray(Vector *a) {
    a->array = malloc(VECTOR_INIT_SIZE * sizeof(void*));
    a->used = 0;
    a->size = VECTOR_INIT_SIZE;
}

// prova ad aggiungere un elemento all'array, se non c'è spazio
// rialloca l'array raddoppiandone la dimensione
void insertArray(Vector *a, void* data) {
    if (a->used == a->size) {
        a->size *= 2;
        a->array = realloc(a->array, a->size * sizeof(void*));
    }
    a->array[a->used++] = data;
}

// trova un elemento in un array
int findArray(Vector *a, void* data) {
    for(int i = 0; i < a->used; i++) {
        if(a->array[i] == data)
            return i;
    }
    return -1;
}

// rimuove un elemento dall'array
void removeArray(Vector *a, void* data) {
    int index = findArray(a, data);
    int last_used_index = --a->used;
    a->array[index] = a->array[last_used_index];
}

void freeArray(Vector *a) {
    free(a->array);
    a->array = NULL;
    a->used = a->size = 0;
}

// void freeVector(Vector **a){
//     free(a);
// }

int getArrayLength(Vector *a){
    return (a->used);
}
