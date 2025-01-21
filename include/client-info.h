#ifndef CLIENT_INFO_H
#define CLIENT_INFO_H

#include "constants.h"
#include "state.h"
#include "vector.h"

typedef struct ClientInfo {
    char username[USERNAME_LEN];
    int score[NUMBER_THEMES];
    int question;
    int themes[NUMBER_THEMES];      
    int socket;
    State state;
} ClientInfo;

// array themes contiene informazioni sui temi che il client ha completato
// 1    tema attivo
// -1   tema completato
// 0    tema non iniziato

extern Vector clients;

ClientInfo* createClient();

void setClientName(ClientInfo*, char*);

int findClientTheme(ClientInfo*);

int countAvailableThemesForClient(ClientInfo*);

void setClientTheme(ClientInfo*, int);
void completedClientTheme(ClientInfo*);
void freeClient(ClientInfo*);

/* Funzioni per la manipolazione dei vettori */
void printArrayUsername(Vector *);
int findArrayUsername(Vector *, char *);
int findArraySocket(Vector *, int);

#endif
