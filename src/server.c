#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include "../include/theme.h"
#include "../include/constants.h"
#include "../include/vector.h"
#include "../include/client-info.h"
#include "../include/msg-exchange.h"
#include "../include/state.h"


Theme *themes[NUMBER_THEMES];
int themes_created;
Vector clients;

int main(int argc, char** argv) {
    createTheme("storia");
    createTheme("generale");
    createTheme("geografia");

    initArray(&clients);

    char recv_buffer[BUF_LEN]; 
    char send_buffer[BUF_LEN];
    char ranking_buffer[LONG_MSG_LEN];

    strcpy(recv_buffer, "");    // pulizia
    strcpy(send_buffer, "");    // pulizia
    strcpy(ranking_buffer, "");    // pulizia

    int ret, newfd, listener;
    unsigned int addrlen;

    fd_set master; // Set principale gestito dal programmatore con le macro
    // socket da monitorare

    fd_set read_fds; // Set di lettura gestito dalla select

    int fdmax; // Numero max di descrittori

    struct sockaddr_in sv_addr, cl_addr; // Indirizzo server

    /* Azzero i set */
    FD_ZERO(&master);
    FD_ZERO(&read_fds);
    listener = socket(AF_INET, SOCK_STREAM, 0); // socket bloccante
    sv_addr.sin_family = AF_INET;
    // INADDR_ANY mette il server in ascolto su tutte le
    // interfacce (indirizzi IP) disponibili sul server
    sv_addr.sin_addr.s_addr = INADDR_ANY;
    sv_addr.sin_port = htons(1234);
    ret = bind(listener, (struct sockaddr *)&sv_addr, sizeof(sv_addr));
    if(ret < 0){
        perror("Error on bind:"); 
        exit(-1);
    }
    listen(listener, 10);
    FD_SET(listener, &master);  // Aggiungo il listener al set
    FD_SET(STDIN_FILENO, &master); // 0 is the file descriptor for stdin
    fdmax = (listener > STDIN_FILENO) ? listener : STDIN_FILENO;           // Tengo traccia del maggiore
    int break_loop = 0;
    printf("Trivia Quiz\n");
    printf("++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n");
    printf("Temi\n");
    for(int i = 0; i < NUMBER_THEMES; i++){
        printf("%d - %s\n", (i+1), themes[i]->name);
    }
    for (;;) {
        if(break_loop)
            break;
        // print stuff
        printf("++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n");
        printArrayUsername(&clients);
        printRankings();
        printThemesCompleted();
        printf("++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n");
        strcpy(recv_buffer, "");    // pulizzia
        strcpy(send_buffer, "");    // pulizzia
        read_fds = master;      // read_fds sarà modificato dalla select
        select(fdmax + 1, &read_fds, NULL, NULL, NULL); 
        // la select ha lasciato in read_fds solo i socket pronti in lettura
        for (int socket = 0; socket <= fdmax; socket++) {  // Scorro tutto il set
            if (FD_ISSET(socket, &read_fds)) { // Trovato un desc pronto
                if(socket == STDIN_FILENO){
                    getLine(recv_buffer);
                    if(strcmp(recv_buffer, "quit") == 0){
                        printf("Quitting\n");
                        break_loop = 1; 
                    } else {
                        printf("Digitare: 'quit' per terminare il server\n");
                    }
                    continue;
                }
                if (socket == listener) {        // È il listener
                    addrlen = sizeof(cl_addr);
                    newfd = accept(listener, (struct sockaddr *)&cl_addr, &addrlen);
                    FD_SET(newfd, &master); // Aggiungo il nuovo socket
                    if (newfd > fdmax) {
                        fdmax = newfd;
                    } // Aggiorno max
                } else {
                    // È un altro socket (non il listener)
                    ret = recv_msg(recv_buffer, socket);
                    if(ret < 0)
                        strcpy(recv_buffer, END_QUIZ_CMD);

                    // registro un nuovo client
                    if (strcmp(recv_buffer, "register") == 0) {
                        ClientInfo* new_client;
                        new_client = createClient();
                        insertArray(&clients, new_client);
                        strcpy(send_buffer, "registered");
                        send_msg(send_buffer, socket);
                        new_client->state = CHOOSING_NAME;
                        new_client->socket = socket;
                        continue;
                    }

                    // trovo il client associato al socket che è pronto
                    int current_client_index = findArraySocket(&clients, socket);
                    if(current_client_index < 0) {
                        perror("Client socket not found: \n");
                        continue;
                    }

                    ClientInfo* current_client = (ClientInfo*)clients.array[current_client_index];
                    // mando al client le classifiche se ha inviato show score
                    if(strcmp(recv_buffer, SHOW_SCORE_CMD) == 0) {
                        char *ranking_buffer = getRankings();
                        send_msg(ranking_buffer, socket);
                        free(ranking_buffer);
                        continue;
                    }

                    // chiudo il socket ed elimino informazioni sul client se ha inviato endquiz
                    if(strcmp(recv_buffer, END_QUIZ_CMD) == 0) {
                        close(socket);           // Chiudo socket
                        FD_CLR(socket, &master); // Rimuovo il socket dal set
                        printf("Il client %s si è disconnesso.\n", current_client->username);
                        // rimuovo il client dalle classifiche e le riordino
                        removeClientRankings(current_client);
                        sortRankings();     
                        // rimuovo il client dall'array globale
                        removeArray(&clients, current_client); 
                        freeClient(current_client);
                        continue;
                    }
                    
                    // il client ha inviato il nome
                    if(current_client->state == CHOOSING_NAME) {
                        // controllo che non ci sia un client con lo stesso username
                        if(findArrayUsername(&clients, recv_buffer) == -1) {
                            strcpy(send_buffer, "name-set");
                            strcpy(current_client->username, recv_buffer);
                            current_client->state = REQUEST_THEME;
                            printArrayUsername(&clients);
                        }
                        else {
                            strcpy(send_buffer, "name-taken");
                        }
                        send_msg(send_buffer, socket);
                        continue;
                    }

                    // il client ha richiesto i temi disponibili
                    if(current_client->state == REQUEST_THEME) {
                        int number_themes = countAvailableThemesForClient(current_client);
                        // mando al client il numero di temi che può ricevere
                        sprintf(send_buffer, "%d", number_themes);
                        send_msg(send_buffer, socket);
                        for(int i = 0; i < NUMBER_THEMES; i++){
                            // non invio temi che ha già completato
                            if(current_client->themes[i] != 0)
                                continue;
                            strcpy(send_buffer, themes[i]->name);
                            send_msg(send_buffer, socket);
                        }
                        current_client->state = CHOOSING_THEME;
                        continue;
                    }
                    
                    // il client ha mandato il tema scelto
                    if(current_client->state == CHOOSING_THEME) {
                        // ottengo l'indice nell'array global themes del tema scelto dal client
                        int theme_index = getThemeIndex(recv_buffer);
                        if(theme_index < 0){
                            perror("Theme not found\n");
                            continue;
                        } 
                        // aggiorno il tema del client
                        Theme* current_theme = themes[theme_index];
                        setClientTheme(current_client, theme_index);
                        //aggiungo il client alla classifica del tema scelto
                        addClientThemeRanking(current_theme, current_client);
                        // aggiorno lo stato del client
                        current_client->state = ANSWERING;
                        current_client->question = 0;
                    }

                    if(current_client->state == ANSWERING) {
                        int current_theme_index = findClientTheme(current_client);
                        // il client ha già risposto ad una domanda controllo che la risposta sia corretta
                        // e informo il client
                        if(current_client->question > 0){
                            Theme* current_theme = themes[current_theme_index];
                            int current_question = current_client->question-1;
                            if(isCorrectAnswer(current_theme_index, current_question, recv_buffer) > 0) {
                                current_client->score[current_theme_index]++;
                                strcpy(send_buffer, "Risposta corretta");
                                sortRanking(current_theme);
                            } else {
                                strcpy(send_buffer, "Risposta errata");
                            }
                            send_msg(send_buffer, socket);
                            // se il client ha risposto a tutte le domande aggiorno il vettore theme del client
                            // e ne aggiorno anche lo stato
                            if(current_client->question == NUMBER_QUESTIONS){
                                current_client->state = REQUEST_THEME;
                                completedClientTheme(current_client);
                                continue;
                            }
                        } 
                        // mando al client la prossima domanda
                        strcpy(send_buffer, themes[current_theme_index]->questions[current_client->question]);
                        send_msg(send_buffer, socket);
                        current_client->question++; 
                        continue;
                    }
                }
            }
        }
    }
    // dealloco i client e il vector
    for(int i = 0; i < clients.used; i++){
        ClientInfo *client = (ClientInfo*)clients.array[i];
        freeClient(client);
    }
    freeArray(&clients);
    // dealloco i themes e il vector
    for(int i = 0; i < NUMBER_THEMES; i++){
        Theme* theme = themes[i];
        freeTheme(theme);
    }
    close(listener);
    printf("Shut Down\n");
    return 0;
}
