#include "./../include/rmpi.h"

static char log_buffer[LOG_BUFFER_SIZE];
extern char rmpi_receiver_shutdown;

typedef struct params {
  int sock;
  rmpi_comm_t comm;
} params_t;

void *rmpi_receiver(void *args) {
  rmpi_receiver_args_t *p = (rmpi_receiver_args_t *)args;
  ssize_t bytes = 0;
  fd_set descriptors;
  int retval = -1;
  int connection = -1;
  struct timespec timeouts;
  struct sockaddr_in client_addr;
  socklen_t client_addr_length = sizeof(client_addr);
  memset(&client_addr, 0, client_addr_length);
  rmpi_pack_t pack;
  char *msg = (char *)malloc(RMPI_MAX_TRANSFER_UNIT);
  if (!msg) {
    rmpi_log_fatal("Cannot allocate memory for receiver");
  }
  rmpi_log_sys("Started received thread");
  timeouts.tv_sec = 2;
  timeouts.tv_nsec = 0;
  struct sockaddr_in connection_addr;
  unsigned int len = sizeof(connection_addr);
  int32_t s = p->comm->mysock;
  while (!p->comm->rmpi_receiver_shutdown) {
    FD_ZERO(&descriptors);
    FD_SET(s, &descriptors);
    retval = pselect(s + 1, &descriptors, NULL, NULL, &timeouts, NULL);

    if (retval) {
      if ((connection =
               accept(s, (struct sockaddr *)&connection_addr, &len)) < 0) {
      } else {
        sprintf(log_buffer, "TCP client conneted, ip: %s",
                inet_ntoa(connection_addr.sin_addr));
        params_t par;
        par.sock = connection;
        par.comm =  p->comm;
        rmpi_log_info(log_buffer);
        pthread_t pid = -1;
        pthread_create(&pid, NULL, rmpi_client_tcp_connection, &par);
        pthread_detach(pid);
      }
    }
  }

  return NULL;
}

int32_t rmpi_recv(void *msg, int64_t count, size_t type, int64_t src,
                  int64_t tag, rmpi_comm_t comm) {
  if (!comm) {
    rmpi_log_fatal("Communicator is NULL");
  }
  int64_t index = rmpi_get_index_by_rank(src, comm);

  if (index < 0) {
    return -1;
  }

  struct sockaddr_in client_addr;
  socklen_t client_addr_length = sizeof(client_addr);
  memset(&client_addr, 0, client_addr_length);
  int32_t current_try = 0;
  ssize_t bytes = 0;
  size_t size = type * count + sizeof(int64_t);
  rmpi_pack_t p;
  void *message = NULL;

  while (!message) message = rmpi_queue_pop(comm->recvq, src);
  memcpy(msg, message, size);

  return 0;
}

void *rmpi_client_tcp_connection(void *args) {
  rmpi_pack_t pack;
  char *msg = (char *)malloc(RMPI_MAX_TRANSFER_UNIT);
  if (!msg) {
    rmpi_log_fatal("Cannot allocate memory for receiver");
  }
  int sh = 0;
  int retval = -1;
  int n = 0;
  int tries = 0;
  ssize_t bytes = 0;
  fd_set descriptors;
  struct timespec timeouts;
  int sock = ((params_t *)args)->sock;
  rmpi_comm_t comm = ((params_t *)args)->comm;

  sprintf(log_buffer, "Processing socket %d", sock);
  rmpi_log_info(log_buffer);
  timeouts.tv_sec = 1;
  timeouts.tv_nsec = 0;

  while (!sh) {
    FD_ZERO(&descriptors);
    FD_SET(sock, &descriptors);
    retval = pselect(sock + 1, &descriptors, NULL, NULL, &timeouts, NULL);

    if (retval < 0 && errno != EINTR) {
      close(sock);
      rmpi_log_fatal("Failed to pselect from socket");
    } else if (retval && errno != EINTR) {
      bytes = recvfrom(sock, &pack, RMPI_MAX_TRANSFER_UNIT, MSG_DONTWAIT, NULL, NULL);
      if (bytes > 0) {
        memcpy(msg, pack.message, bytes - sizeof(int64_t));
        rmpi_queue_push(comm->recvq, msg, pack.rank, bytes);
        sprintf(log_buffer, "Rank %d received message with size %d from %d",
                comm->myrank, bytes - sizeof(int64_t), pack.rank);
        rmpi_log_info(log_buffer);
        bytes = sendto(sock, &bytes, sizeof(bytes), MSG_DONTWAIT, NULL, -1);
      }
    } else sh = 1;
  }

  rmpi_log_sys("Stopping TCP connection...");
  close(sock);
  rmpi_log_sys("TCP Connection Terminated");
  return NULL;
}
