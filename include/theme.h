#ifndef THEME_H
#define THEME_H

#include "constants.h"
#include "vector.h"
#include "client-info.h"

typedef struct Theme {
    char *name;
    char *file_path;
    char *questions[NUMBER_QUESTIONS];
    Vector answers[NUMBER_QUESTIONS];
    Vector ranking;
} Theme;

extern Theme *themes[NUMBER_THEMES];
 
extern int themes_created;

void createTheme(char*);

void freeTheme(Theme*);

int getThemeIndex(char*);

int isCorrectAnswer(int, int, char*);

void addClientThemeRanking(Theme *, ClientInfo*);

void removeClientThemeRanking(Theme *, ClientInfo*);

void removeClientRankings(ClientInfo*);

void freeRankings(Theme *);

void sortRanking(Theme *t);

void sortRankings();

void printRanking(Theme *t);

void printRankings();

char* getRanking(Theme *t);

char* getRankings();

void printThemeCompleted(Theme *t);

void printThemesCompleted();

#endif
