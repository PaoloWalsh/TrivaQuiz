#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../include/client-info.h"

ClientInfo* createClient(){
    ClientInfo* client = malloc(sizeof(ClientInfo));
    strcpy(client->username, "unset-username");
    client->question = 0;
    memset(client->score, 0, sizeof(client->score));
    client->state = CHOOSING_NAME;
    memset(client->themes, 0, sizeof(client->themes));
    return client;
}

void setClientName(ClientInfo *client, char *name){
    strcpy(client->username, name);
}

// restituisce l'indice del tema attivo di un client
int findClientTheme(ClientInfo* client) {
    for(int i = 0; i < NUMBER_THEMES; i++) {
        if(client->themes[i] == 1)
            return i;   // indice del tema attivo
    }
    return -1;  // il client non ha temi attivi
}

// conta il numero di temi che non client non ha ancora iniziato
int countAvailableThemesForClient(ClientInfo* client){
    int ret = 0;
    for(int i = 0; i < NUMBER_THEMES; i++) {
        if(client->themes[i] == 0)
            ret++;
    }
    return ret;
}

// imposta come attivo un tema per un client
void setClientTheme(ClientInfo* client, int theme) {
    int warning = findClientTheme(client);
    if(warning != -1)
        printf("WARNING Client %s\n, has this %d\n active theme", client->username, warning);
    client->themes[theme] = 1;
}

// imposta come completato il tema attivo di un client
void completedClientTheme(ClientInfo* client) {
    int warning = -1;
    for(int i = 0; i < NUMBER_THEMES; i++) {
        if(client->themes[i] == 1){
            client->themes[i] = -1;
            warning++;
        }
    }
    if(warning != 0)
        printf("Warning in setting theme as completed!\n");
}

void freeClient(ClientInfo* client) {
    free(client);
}

/* Functions for the client-info array */

// stampa i nomi dei vari client collegati 
void printArrayUsername(Vector *a){
    if(a->used == 0){
        printf("\nNon ci sono client collegati\n");
        return;
    }
    printf("\nI client attualmente collegati sono (%d):\n", a->used);
    for(int i = 0; i < a->used; i++){
        ClientInfo *c = (ClientInfo*)a->array[i];
        printf("\t%d. %s\n", (i+1),c->username);
    }
}

// trova un client in un vector in base al suo username
int findArrayUsername(Vector *a, char *username){
    for(int i = 0; i < a->used; i++) {
        ClientInfo *c = (ClientInfo*)a->array[i];
        if(strcmp(c->username, username) == 0)
            return i;
    }
    return -1;
}

// trova un client in un vector in base al suo socket
int findArraySocket(Vector *a, int socket){
    for(int i = 0; i < a->used; i++) {
        ClientInfo *c = (ClientInfo*)a->array[i];
        if(c->socket == socket)
            return i;
    }
    return -1;
}
