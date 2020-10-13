#ifndef M_SERVER_H
#define M_SERVER_H

#define _GNU_SOURCE
#include <arpa/inet.h>
#include <errno.h>
#include <fcntl.h>
#include <netdb.h>
#include <netinet/in.h>
#include <pthread.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include "./Mlogger.h"

#define MU 256
#define MAX(x, y) (x > y ? x : y)
#define MAX_TRIES 60

int startMultiServer();
void parseParams(int argc, char **argv);
void sighandler(int s, siginfo_t *info, void *param);
void *clientTCPConnection(void *args);
int acceptConnection(int sTCP, int sUDP);

#endif