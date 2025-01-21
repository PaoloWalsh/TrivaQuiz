#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <signal.h>

#include "../include/constants.h"
#include "../include/state.h"
#include "../include/client-messages.h"
#include "../include/msg-exchange.h"

// contiene il nome dei temi inviati dal server
char* theme_names[NUMBER_THEMES];
// contiene il numero di temi inviati da server
int number_themes;

int main(int argc, char** argv) {
    signal(SIGPIPE, SIG_IGN);
    // indice tema attivo
    int active_theme = -1;
    // domanda a cui il client sta rispondendo
    int question = 0;

    State state = UNREGISTERED;

    // oggetto per la gestione dei messaggi stampati all'utente
    Cmess cmess;

    createThemeNames();

    int choice;
    char recv_buffer[BUF_LEN]; 
    char send_buffer[BUF_LEN];
    // memorizza l'ultima doamnda mandata dal server nel caso in cui
    // il client stia rispondendo alle domande
    char last_question[BUF_LEN];

    strcpy(recv_buffer, "");    // pulizia
    strcpy(send_buffer, "");    // pulizia
    strcpy(last_question, "");    // pulizia

    int ret, sd;
    struct sockaddr_in server_addr; // Struttura per il server
    /* Creazione socket */
    char *ip = "127.0.0.1";
    int porta = (argc > 1) ? atoi(argv[0]) : 1234;

    for(;;) {

        strcpy(send_buffer, "");

        if(state == UNREGISTERED){

            // stampo il messaggio di greeting
            loadCmessGreeting(&cmess);
            showCmess(&cmess);

            getLine(send_buffer);
            choice = atoi(send_buffer);

            if(choice != 1 && choice != 2){
                printf("Input non valido\n");
                continue;
            }

            if (choice != 1) {
                printf("Quitting\n");
                break;
            }

            // se il client ha scelto 1 mi connetto al server
            sd = socket(AF_INET, SOCK_STREAM, 0);
            /* Creazione indirizzo del server */
            memset(&server_addr, 0, sizeof(server_addr)); // Pulizia
            server_addr.sin_family = AF_INET;
            server_addr.sin_port = htons(porta);
            inet_pton(AF_INET, ip, &server_addr.sin_addr);
            ret = connect(sd, (struct sockaddr *)&server_addr, sizeof(server_addr));
            if (ret == -1) {
                perror("Errore sulla connect\n");
                close(sd);
                return 1;
            }
            strcpy(send_buffer, "register");
        }

        else if(state == REQUEST_THEME) {
            strcpy(send_buffer, "what-themes"); 
        } 

        else {  // copre i seguenti stati: CHOOSING_NAME, CHOOSING_THEME, ANSWERING, comands
            getLine(send_buffer);
        }

        // se sto rispondendo ad una domanda incremento il contatore
        if(state == ANSWERING && strcmp(send_buffer, SHOW_SCORE_CMD) != 0)
            question++;

        // se sto scegliendo il tema copio in send buffer il nome del tema e non l'indice dato che l'array del server
        // potrebbe contenere più temi e quindi gli indici potrebbero non coincidere
        if(state == CHOOSING_THEME && strcmp(send_buffer, END_QUIZ_CMD) != 0 && strcmp(send_buffer, SHOW_SCORE_CMD) != 0) {
            active_theme = atoi(send_buffer);
            active_theme--;            
            if(active_theme < 0 || active_theme >= number_themes) {
                printf("Input non valido\n");
                continue;
            }
            strcpy(send_buffer, theme_names[active_theme]);
            state = ANSWERING;
        }

        strcpy(recv_buffer, "");
        ret = send_msg(send_buffer, sd);

        if(ret < 0){
            printf("Server si è disconnesso\n");
            break;
        }

        // se il client ha mandato endquiz riparto come UNREGISTERED
        if(strcmp(send_buffer, END_QUIZ_CMD) == 0){
            question = 0;
            state = UNREGISTERED;
            strcpy(last_question, "");
            close(sd);
            continue;
        }

        ret = recv_msg(recv_buffer, sd);

        if(ret < 0){
            printf("Server si è disconnesso\n");
            break;
        }

        // se ho ricevuto le classifiche le stampo
        if(strcmp(send_buffer, SHOW_SCORE_CMD) == 0){
            printf("%s\n", recv_buffer);
        }

        // se ho ricevuto registered passo allo stato CHOOSING_NAME
        if(strcmp(recv_buffer, "registered") == 0) {
            printf("Client registered\n"); 
            state = CHOOSING_NAME;
        }

        // se ho ricevuto name-taken devo rinviare il nome
        // quindi non aggiorno lo stato
        if(strcmp(recv_buffer, "name-taken") == 0) {
            printf("Username Taken\n");
            continue;
        }

        // se il nome è stato settato aggiorno lo stato a REQUEST_THEME
        if(strcmp(recv_buffer, "name-set") == 0) {
            printf("Username Set\n"); 
            state = REQUEST_THEME;
            continue;
        }

        // se lo stato è CHOOSING_NAME stampo il messaggio all'utente chiedendo di inserire
        // un nome univoco
        if(state == CHOOSING_NAME){
            loadCmessUsername(&cmess);
            showCmess(&cmess);
            continue;
        } 

        // se lo stato è REQUEST_THEME faccio altre recv_msg dato che il server
        // mi sta inviando i nomi dei temi
        if(state == REQUEST_THEME) {
            number_themes = atoi(recv_buffer);
            // riempio il vettore theme_names con i nomi che il server mi invia
            for(int i = 0; i < number_themes; i++){
                ret = recv_msg(recv_buffer, sd);
                if(ret < 0)
                    continue;
                strcpy(theme_names[i], recv_buffer);
            }
            state = CHOOSING_THEME;
        }

        // stampo i temi disponibili inviati dal server
        if(state == CHOOSING_THEME) {
            loadCmessThemes(&cmess);
            showCmess(&cmess);
            continue;
        }

        // sto rispondendo alle domande
        if(state == ANSWERING) {
            // se non ho mandato show score allora ho mandato
            // la risposta ad una domanda
            if(strcmp(send_buffer, SHOW_SCORE_CMD) != 0){
                // se question == 0 in recv_buffer c'è una domanda
                // e non l'esito della risposta precedente
                if(question == 0){
                    strcpy(last_question, recv_buffer);
                } 
                // in recv_buffer c'è l'esito della risposta precedente
                // lo stampo e faccio un'altra recv_msg per ottere la domanda successiva
                else{
                    printf("%s\n", recv_buffer);
                    // se ho finito il tema faccio nuovamente la richiesta dei temi
                    if(question == NUMBER_QUESTIONS){   // tema completato
                        state = REQUEST_THEME;
                        question = 0;
                        printf("Tema Completato!\n");
                        resetThemeNames();
                        continue;
                    }
                    // ricevo prossima domanda e la salvo in last_question
                    recv_msg(recv_buffer, sd);
                    strcpy(last_question, recv_buffer);
                }
            }
            // stampo domanda ricevuta
            loadCmessQuestion(&cmess, active_theme);
            showCmess(&cmess);
            printf("%s", last_question);
        }
    }

    close(sd);
    freeThemeNames();
    printf("Shut down\n");
    return 0;
}
