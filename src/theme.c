#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>
#include "../include/theme.h"
#include "../include/constants.h"


// crea un tema caricando le domande e risposte in vettori e aggiunge il tema al vettore mantenuto dal server
void createTheme(char* name) {
    if(themes_created >= NUMBER_THEMES)
        return;
    Theme* theme = (Theme*)malloc(sizeof(Theme));
    theme->name = malloc(BUF_LEN);
    strcpy(theme->name, name);
    theme->file_path = malloc(BUF_LEN);
    strcpy(theme->file_path, "txt/");
    strcat(theme->file_path, theme->name);
    strcat(theme->file_path, ".txt");
    initArray(&theme->ranking);
    // aggiungo il tema al vettore che contiene i nomi dei temi
    themes[themes_created] = theme;
    themes_created++;

    FILE* file;
    if ((file = fopen(theme->file_path,"r")) == NULL){
        printf("Error! opening file\n");
        exit(1);
    }
    for(int i = 0; i < NUMBER_QUESTIONS; i++){
        theme->questions[i] = malloc(BUF_LEN);
        // creo un vector di risposte corrette per ogni domanda
        initArray(&theme->answers[i]);
        char string[BUF_LEN];
        fgets(string, sizeof(string), file);
        strcpy(theme->questions[i], string);
        fgets(string, sizeof(string), file);
        int number_answers = atoi(string);
        for(int j = 0; j < number_answers; j++) {
            fgets(string, sizeof(string), file);
            char* answer = malloc(BUF_LEN);
            strcpy(answer, string);
            insertArray(&theme->answers[i], answer);
        }
    }
    fclose(file);
}

// dato il nome di un tema restituisce l'indice del tema all'interno del vettore mantenuto dal server
int getThemeIndex(char* theme_name) {
    for(int i = 0; i < NUMBER_THEMES; i++){
        if(strcmp(themes[i]->name, theme_name) == 0)
            return i;
    }
    return -1;
}

// dealloca memoria usata dal tema
void freeTheme(Theme* theme) {
    free(theme->name);
    free(theme->file_path);
    for(int i = 0; i < NUMBER_QUESTIONS; i++){
        for(int j = 0; j < theme->answers[i].used; j++)
            free(theme->answers[i].array[j]);
        freeArray(&theme->answers[i]);
        free(theme->questions[i]);
    }
    freeArray(&theme->ranking);
    free(theme);
}

// funzione di utilità per eliminare il carattere '\n' se presente in fondo alla stringa
void removeNewLineChar(char *string) {
    if (string[strlen(string) - 1] == '\n') {
        string[strlen(string) - 1] = '\0';
    }
}

// verifica che la risposta dell'utente sia uguale a una delle risposte possibili per la domanda in esame
int isCorrectAnswer(int theme_index, int question_index, char *user_answer){
    removeNewLineChar(user_answer);
    Theme *current_theme = themes[theme_index];
    for(int i = 0; i < current_theme->answers[question_index].used; i++){
        char* answer = (char*)current_theme->answers[question_index].array[i];
        removeNewLineChar(answer);
        // converte le stringe a lettere minuscole in modo tale da fare un confronto case insensitive
        int j = 0;
        while (answer[j]) {
            answer[j] = tolower(answer[j]);
            j++;
        }
        j = 0;
        while (user_answer[j]) {
            user_answer[j] = tolower(user_answer[j]);
            j++;
        }
        if(strcmp(answer, user_answer) == 0){
            return 1;
        } 
    }
    return -1;
}

/* Funzioni per la manipolazioni delle classifiche */

void addClientThemeRanking(Theme *t, ClientInfo* client){
    insertArray(&t->ranking, (void*)client);
}

void removeClientThemeRanking(Theme *t, ClientInfo* client){
    removeArray(&t->ranking, (void*)client);
}

// rimuove i client dalle classifiche, chiamata quando un client si disconnette
void removeClientRankings(ClientInfo* client){
    for(int i = 0; i < NUMBER_THEMES; i++){
        Theme* current_theme = themes[i];
        if(client->themes[i] != 0)  // rimuovo se il tema è completato o iniziato
            removeClientThemeRanking(current_theme, client);
    }
}

// ordina la classifica di un tema in base al punteggio dei client in quel tema
void sortRanking(Theme *t){
    int length = getArrayLength(&t->ranking);
    int theme_index = getThemeIndex(t->name);
    for(int i = 0; i < length; i++){
        ClientInfo *max_client = (ClientInfo*)t->ranking.array[i];
        int max_client_index = i;
        for(int j = i+1; j < length; j++) {
            ClientInfo *current_client = (ClientInfo*)t->ranking.array[j];
            if(max_client->score[theme_index] < current_client->score[theme_index]){
                max_client = current_client;
                max_client_index = j;
            }
        }
        ClientInfo *temp_client = t->ranking.array[i];
        t->ranking.array[i] = (void*)max_client;
        t->ranking.array[max_client_index] = temp_client;

    }
}

// ordina tutte le classifiche
void sortRankings() {
    for(int i = 0; i < NUMBER_THEMES; i++){
        Theme* current_theme = themes[i];
        sortRanking(current_theme);
    }
}

// stampa la classifica
void printRanking(Theme *t){
    printf("\n");
    printf("Tema - ");
    printf("%s", t->name);
    printf(" - Classifica:\n");
    int theme_index = getThemeIndex(t->name);
    int length = getArrayLength(&t->ranking);
    if(length == 0){
        printf("\tClassifica vuota\n");
    }
    for(int i = 0; i < length; i++) {
        ClientInfo* current_client = t->ranking.array[i];
        printf("\t%d. %s, punteggio: %d\n", i+1, current_client->username, current_client->score[theme_index]);
    }
    printf("\n");
}

// stampa tutte le classifiche
void printRankings() {
    for(int i = 0; i < NUMBER_THEMES; i++){
        Theme* current_theme = themes[i];
        printRanking(current_theme);
    }
}

// restituisce in una stringa una classifica
// la dimensione del buffer dipende dal numero di client
char* getRanking(Theme *t){
    char *buffer;
    int ranking_lines = t->ranking.used + 1;
    buffer = malloc(RANKING_ROW * ranking_lines);
    strcpy(buffer, "\nTema - ");
    strcat(buffer, t->name);
    strcat(buffer, " - Classifica:\n");
    int theme_index = getThemeIndex(t->name);
    int length = getArrayLength(&t->ranking);
    if(length == 0){
        strcat(buffer, "\tClassifica vuota\n");
    }
    for(int i = 0; i < length; i++) {
        ClientInfo* current_client = t->ranking.array[i];
        char temp_buffer[BUF_LEN];
        sprintf(temp_buffer, "\t%d. %s, punteggio: %d\n", i+1, current_client->username, current_client->score[theme_index]);
        strcat(buffer, temp_buffer);
    }
    strcat(buffer, "\n");
    return buffer;
}

// restituisce in una stringa tutte le classifiche
// la dimensione del buffer dipende dal numero di client e di temi
char* getRankings() {
    char *buffer;
    int ranking_lines = clients.used + 1;
    int total_ranking_lines = NUMBER_THEMES * ranking_lines;
    buffer = malloc(RANKING_ROW * total_ranking_lines);
    strcpy(buffer, "");
    for(int i = 0; i < NUMBER_THEMES; i++){
        Theme* current_theme = themes[i];
        char *temp_buffer = getRanking(current_theme);
        strcat(buffer, temp_buffer);
        free(temp_buffer);
    }
    return buffer;
}

// stampa i client che hanno completato il tema e il loro punteggio
void printThemeCompleted(Theme *t){
    printf("Quiz Tema - %s completato da:\n", t->name);
    int theme_index = getThemeIndex(t->name);
    int length = getArrayLength(&t->ranking);
    int completed_counter = 0;
    for(int i = 0; i < length; i++) {
        ClientInfo* current_client = t->ranking.array[i];
        if(current_client->themes[theme_index] != -1)
            continue;
        printf("\t%d. %s, punteggio: %d\n", i+1, current_client->username, current_client->score[theme_index]);
        completed_counter++;
    }
    if(length == 0 || completed_counter == 0){
        printf("\tNessun client ha ancora completato il tema\n");
    }
    printf("\n");
}


// per ogni tema stampa i client che hanno completato il tema e il loro punteggio
void printThemesCompleted() {
    for(int i = 0; i < NUMBER_THEMES; i++){
        Theme* current_theme = themes[i];
        printThemeCompleted(current_theme);
    }
}
