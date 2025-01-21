#ifndef CMESS_H
#define CMESS_H

#include "constants.h"

typedef struct Cmess {
    char message[LONG_MSG_LEN];
} Cmess;

extern char* theme_names[NUMBER_THEMES];
extern int number_themes;

void createThemeNames();

void freeThemeNames();

void loadCmessGreeting(Cmess*);

void loadCmessThemes(Cmess*);

void loadCmessUsername(Cmess*);

void loadCmessQuestion(Cmess* , int);

void showCmess(Cmess*);

void resetThemeNames();

#endif
