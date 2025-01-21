#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "../include/client-messages.h"

// alloca le stringhe per contenere il nome dei temi che il server manda al client
void createThemeNames() {
    for(int i = 0; i < NUMBER_THEMES; i++){
        theme_names[i] = malloc(USERNAME_LEN); 
    }
}

// dealloca le stringhe 
void freeThemeNames() {
    for(int i = 0; i < NUMBER_THEMES; i++){
        free(theme_names[i]);
    }
}

void loadCmessGreeting(Cmess* cmess){
    strcpy(cmess->message, "");
    strcpy(cmess->message, "\nTrivia Quiz\n");
    strcat(cmess->message, "++++++++++++++++++++++++++++++++++++++++++++\n");
    strcat(cmess->message, "Menù:\n");
    strcat(cmess->message, "1 - Comincia una sessione di Triva\n");
    strcat(cmess->message, "2 - Esci\n");
    strcat(cmess->message, "++++++++++++++++++++++++++++++++++++++++++++\n");
    strcat(cmess->message, "La tua scelta:\n");
}

void loadCmessThemes(Cmess* cmess){
    strcpy(cmess->message, "");
    char temp_buffer[BUF_LEN];
    strcat(cmess->message, "\nQuiz disponibili:\n");
    strcat(cmess->message, "++++++++++++++++++++++++++++++++++++++++++++\n");
    if(number_themes == 0)
        strcat(cmess->message, "Hai completato tutti i temi disponibili!\n");
    for(int i = 0; i < number_themes; i++){
        snprintf(temp_buffer, BUF_LEN, "%d - ", (i+1));
        strcat(temp_buffer, theme_names[i]);
        strcat(temp_buffer, "\n");
        strcat(cmess->message, temp_buffer);
    }
    strcat(cmess->message, "++++++++++++++++++++++++++++++++++++++++++++\n");
    strcat(cmess->message, "La tua scelta:\n");
}

void loadCmessUsername(Cmess* cmess) {
    strcpy(cmess->message, "");
    strcpy(cmess->message, "\nTrivia Quiz\n");
    strcat(cmess->message, "++++++++++++++++++++++++++++++++++++++++++++\n");
    strcat(cmess->message, "Scegli un nickname (deve essere univoco):\n");
}

void loadCmessQuestion(Cmess* cmess, int theme_index) {
    strcpy(cmess->message, "");
    strcpy(cmess->message, "\nQuiz - ");
    strcat(cmess->message, theme_names[theme_index]);
    strcat(cmess->message, "\n");
    strcat(cmess->message, "++++++++++++++++++++++++++++++++++++++++++++\n");
}

void showCmess(Cmess* cmess){
    printf("%s", cmess->message);
}

// resetta tutti i nomi dei temi e il contatore dei temi che il server manda al client
void resetThemeNames(){
    for(int i = 0; i < number_themes; i++){
        strcpy(theme_names[i], "");
    }
    number_themes = 0;
}
