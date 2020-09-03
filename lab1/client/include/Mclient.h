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

int startUDPClient();
void parseParams(int argc, char **argv);

#endif