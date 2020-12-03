#ifndef M_RMPI_H
#define M_RMPI_H

#define _GNU_SOURCE
#include <arpa/inet.h>
#include <errno.h>
#include <fcntl.h>
#include <netdb.h>
#include <netinet/in.h>
#include <pthread.h>
#include <rpc/rpc.h>
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

#include "./rmpi_logger.h"

/* Define remote program number and version */
#define RMTPROGNUM (u_long)0x3fffffffL
#define RMTPROGVER (u_long)0x1
#define RMTPROGPROC (u_long)0x1
#define RMTTRIESLIMIT 5
#define RMTTRIESSLEEP 200000

#define RMPI_INIT_SLEEP 2

#define RMPI_SEND_TIMEOUT 10
#define RMPI_SEND_TRIES 5
#define RMPI_RECV_TIMEOUT (RMPI_SEND_TIMEOUT * (RMPI_SEND_TRIES + 1))

#define RMPI_COMM_WORLD rmpi_get_comm_world()
#define RMPI_PROC_NULL -1

typedef struct {
  uint16_t port;
  char *addr;
} host_t;

typedef struct {
  int64_t rank;
  int32_t sock;
  host_t host;
  struct sockaddr_in server_addr;
  socklen_t server_addr_length;
} hosts_list_t;

typedef struct {
  int64_t commsize;
  int64_t myrank;
  int32_t mysock;
  hosts_list_t *hosts;
} * comm_t;

#define RMPI_TYPE_CHAR sizeof(signed char)
#define RMPI_TYPE_SHORT sizeof(signed short int)
#define RMPI_TYPE_INT sizeof(signed int)
#define RMPI_TYPE_LONG sizeof(signed long int)
#define RMPI_TYPE_UNSIGNED_CHAR sizeof(unsigned char)
#define RMPI_TYPE_UNSIGNED_SHORT sizeof(unsigned short int)
#define RMPI_TYPE_UNSIGNED_INT sizeof(unsigned int)
#define RMPI_TYPE_UNSIGNED RMPI_TYPE_UNSIGNED_INT
#define RMPI_TYPE_UNSIGNED_LONG sizeof(unsigned long int)
#define RMPI_TYPE_FLOAT sizeof(float)
#define RMPI_TYPE_DOUBLE sizeof(double)
#define RMPI_TYPE_LONG_DOUBLE sizeof(long double)

int32_t rmpi_bcast(void *msg, int64_t count, size_t type, int64_t src, int64_t tag,
               comm_t comm);
int32_t rmpi_send(void *msg, int64_t count, size_t type, int64_t dst, int64_t tag,
              comm_t comm);
int32_t rmpi_recv(void *msg, int64_t count, size_t type, int64_t src, int64_t tag,
              comm_t comm);
void rmpi_init(int *argc, char ***argv, uint8_t debug);
void rmpi_parse_params(int *argc, char ***argv);
int32_t rmpi_register();
void rmpi_finilize();
void *start_svc();
uint16_t rmpi_server();
int32_t rmpi_get_port(char *host, int64_t rank);
void rmpi_create_socket(comm_t comm);
int32_t rmpi_create_server_socket();
int32_t rmpi_create_client_socket(int64_t rank, comm_t comm);
void rmpi_get_ports(comm_t comm);
void rmpi_print_comm(comm_t comm);
comm_t rmpi_get_comm_world();
int64_t rmpi_get_index_by_rank(int64_t rank, comm_t comm);
int64_t rmpi_get_rank(comm_t comm);
int64_t rmpi_get_commsize(comm_t comm);

#endif