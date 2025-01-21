#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include "../include/msg-exchange.h"

// manda un msg contenuto in send_buffer inviando prima la dimensione
int send_msg(char* send_buffer, int socket){
    int len = strlen(send_buffer) + 1;
    uint16_t lmsg = htons(len);
    int ret = send(socket, (void *)&lmsg, sizeof(lmsg), 0);
    if (ret < sizeof(lmsg)){
        return -1;
    }
    ret = send(socket, (void *)send_buffer, len, 0);
    if (ret < len){
        return -1;
    }
    return 1;
}

// riceve un msg in send_buffer ricevendo prima la dimensione
int recv_msg(char* recv_buffer, int socket) {
    int len, ret;
    uint16_t lmsg;
    ret = recv(socket, (void*)&lmsg, sizeof(lmsg), 0);
    if (ret < sizeof(lmsg)) {
        return -1;
    }
    len = ntohs(lmsg);
    ret = recv(socket, (void*)recv_buffer, len, 0);
    if (ret < len) {
        return -1;
    }
    return 1;
}

// funzione di utilità per leggere una riga dallo stdin in maniera sicura
int getLine (char *buffer) {
    if (buffer == NULL) {
        return 0;
    }

    size_t size = BUF_LEN;
    if (fgets(buffer, size, stdin) == NULL) {
        return 0; 
    }

    // rimuovo il carattere \n alla fine della stringa se presente
    if(buffer[strlen(buffer) - 1] == '\n')
        buffer[strlen(buffer) - 1] = '\0';

    return 1;
}
