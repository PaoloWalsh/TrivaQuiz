#ifndef MSG_EXC_H
#define MSG_EXC_H

#include "constants.h"


int send_msg(char* send_buffer, int socket);
int recv_msg(char* recv_buffer, int socket);
int getLine (char *buff);

#endif 
