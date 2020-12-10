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
#define RMPI_RMTPROGNUM (u_long)0x3fffffffL
#define RMPI_RMTPROGVER (u_long)0x1
#define RMPI_RMTPROGPROC (u_long)0x1
#define RMPI_RMTTRIESLIMIT 5
#define RMPI_RMTTRIESSLEEP 200000

#define RMPI_MAX_TRANSFER_UNIT 1000
#define RMPI_INIT_SLEEP 2

#define RMPI_SEND_TIMEOUT 10
#define RMPI_SEND_TRIES 5
#define RMPI_RECV_TIMEOUT (RMPI_SEND_TIMEOUT * (RMPI_SEND_TRIES + 1))

#define RMPI_COMM_WORLD rmpi_get_comm_world()
#define RMPI_PROC_NULL -1

typedef struct {
  uint16_t port;
  char *addr;
} rmpi_host_t;

typedef struct rmpi_hosts_list {
  int64_t rank;
  int32_t sock;
  rmpi_host_t host;
  struct sockaddr_in server_addr;
  socklen_t server_addr_length;
} rmpi_hosts_list_t;

typedef struct rmpi_q_el {
  struct rmpi_q_el *next;
  size_t size;
  int64_t rank;
  void *message;
} * rmpi_q_el_t;

typedef struct rmpi_queue {
  rmpi_q_el_t head;
  rmpi_q_el_t end;
  int64_t len;
  pthread_mutex_t locker;
} * rmpi_queue_t;

typedef struct rmpi_comm {
  int64_t commsize;
  int64_t myrank;
  int32_t mysock;
  pthread_t recvt;
  char rmpi_receiver_shutdown;
  rmpi_hosts_list_t *hosts;
  rmpi_queue_t *recvq;
} * rmpi_comm_t;

typedef struct rmpi_receiver_args {
  rmpi_comm_t comm;
} rmpi_receiver_args_t;

typedef struct {
  int64_t rank;
  char message[RMPI_MAX_TRANSFER_UNIT - sizeof(int64_t)];
} rmpi_pack_t;

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

int32_t rmpi_bcast(void *msg, int64_t count, size_t type, int64_t src,
                   int64_t tag, rmpi_comm_t comm);
int32_t rmpi_send(void *msg, int64_t count, size_t type, int64_t dst,
                  int64_t tag, rmpi_comm_t comm);
int32_t rmpi_recv(void *msg, int64_t count, size_t type, int64_t src,
                  int64_t tag, rmpi_comm_t comm);
void rmpi_init(int *argc, char ***argv, uint8_t debug);
void rmpi_parse_params(int *argc, char ***argv);
int32_t rmpi_register();
void rmpi_finilize();
void *start_svc();
uint16_t rmpi_server();
int32_t rmpi_get_port(char *host, int64_t rank);
void rmpi_create_socket(rmpi_comm_t comm);
int32_t rmpi_create_server_socket();
int32_t rmpi_create_client_socket(int64_t rank, rmpi_comm_t comm);
int32_t rmpi_destroy_client_socket(int64_t rank, rmpi_comm_t comm);
void rmpi_get_ports(rmpi_comm_t comm);
void rmpi_print_comm(rmpi_comm_t comm);
rmpi_comm_t rmpi_get_comm_world();
int64_t rmpi_get_index_by_rank(int64_t rank, rmpi_comm_t comm);
int64_t rmpi_get_rank(rmpi_comm_t comm);
int64_t rmpi_get_commsize(rmpi_comm_t comm);
void *rmpi_receiver(void *args);

rmpi_queue_t rmpi_queue_init();
void rmpi_queue_free(rmpi_queue_t q);
void rmpi_queue_push(rmpi_queue_t q, void *message, int64_t rank, size_t size);
void *rmpi_queue_pop(rmpi_queue_t q, int64_t rank);
#endif