#ifndef M_SERVER_H
#define M_SERVER_H

#include <Mlogger.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

int startUDPServer();
void parseParams(int argc, char **argv);
void sighandler(int s);

#endif