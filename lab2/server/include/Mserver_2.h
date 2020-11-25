#ifndef M_SERVER_H
#define M_SERVER_H

#define _GNU_SOURCE
#include <arpa/inet.h>
#include <fcntl.h>
#include <netdb.h>
#include <netinet/in.h>
#include <errno.h>
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
#include <pthread.h>
#include "./Mlogger_2.h"

#define MU 256

int startTCPServer();
void parseParams(int argc, char **argv);
void sighandler(int s, siginfo_t *info, void *param);
void clientConnection(int sock);
int acceptTCPConnection(int server_socket);

#endif
