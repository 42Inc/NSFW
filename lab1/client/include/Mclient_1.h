#ifndef M_CLIENT_H
#define M_CLIENT_H

#define _GNU_SOURCE
#include "./Mlogger_1.h"
#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>

#define MU 1024
#define MSG_SIZE (MU - 2 * sizeof(unsigned long int))
#define MAXX_TRIES 3

int startUDPClient();
void parseParams(int argc, char **argv);

#endif