#ifndef M_CLIENT_H
#define M_CLIENT_H

#include <Mlogger.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <netdb.h>

#define MU 1024
#define MSG_SIZE (MU - 2 * sizeof(unsigned long int))

int startUDPClient();
void parseParams(int argc, char **argv);

#endif