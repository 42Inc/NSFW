#ifndef M_SERVER_H
#define M_SERVER_H

#include <Mlogger.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>

int startUDPServer();
void parseParams();

#endif