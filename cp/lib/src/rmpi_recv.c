#include "./../include/rmpi.h"

static char log_buffer[LOG_BUFFER_SIZE];
extern char rmpi_receiver_shutdown;

void *rmpi_receiver(void *args) {
  rmpi_receiver_args_t *p = (rmpi_receiver_args_t *)args;
  ssize_t bytes = 0;
  fd_set descriptors;
  int retval = -1;
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
  int32_t s = p->comm->mysock;
  while (!p->comm->rmpi_receiver_shutdown) {
    FD_ZERO(&descriptors);
    FD_SET(s, &descriptors);
    retval = pselect(s + 1, &descriptors, NULL, NULL, &timeouts, NULL);

    if (retval) {
      bytes = recvfrom(s, &pack, RMPI_MAX_TRANSFER_UNIT, MSG_DONTWAIT,
                       (struct sockaddr *)&client_addr, &client_addr_length);
      if (bytes > 0) {
        memcpy(msg, pack.message, bytes - sizeof(int64_t));
        rmpi_queue_push(p->comm->recvq, msg, pack.rank, bytes);
        sprintf(log_buffer, "Rank %d received message with size %d from %d",
                p->comm->myrank, bytes - sizeof(int64_t), pack.rank);
        rmpi_log_info(log_buffer);
        bytes = sendto(s, &bytes, sizeof(bytes), MSG_DONTWAIT,
                       (struct sockaddr *)&client_addr, client_addr_length);
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
  // bytes = recvfrom(comm->mysock, &p, size, MSG_WAITALL,
  //                  (struct sockaddr *)&client_addr, &client_addr_length);
  // if (bytes <= 0) {
  //   return -1;
  // }

  // bytes = sendto(comm->mysock, &bytes, sizeof(bytes), MSG_DONTWAIT,
  //                (struct sockaddr *)&client_addr, client_addr_length);
  // if (bytes <= 0) {
  //   return -1;
  // }
  // memcpy(msg, p.message, type * count);
  return 0;
}
