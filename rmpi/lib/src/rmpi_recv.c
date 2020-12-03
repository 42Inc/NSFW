#include "./../include/rmpi.h"

static char log_buffer[LOG_BUFFER_SIZE];

int32_t rmpi_recv(void *msg, int64_t count, size_t type, int64_t src,
                  int64_t tag, comm_t comm) {
  if (!comm) {
    logFatal("Communicator is NULL");
  }
  sprintf(log_buffer, "Rank %d receiving message with size %d from %d",
          comm->myrank, type * count, src);
  logInfo(log_buffer);

  int64_t index = rmpi_get_index_by_rank(src, comm);

  if (index < 0) {
    return -1;
  }

  struct sockaddr_in client_addr;
  socklen_t client_addr_length = sizeof(client_addr);
  memset(&client_addr, 0, client_addr_length);
  int32_t current_try = 0;
  ssize_t bytes = 0;
  size_t size = type * count;

  bytes = recvfrom(comm->mysock, msg, size, MSG_WAITALL,
                   (struct sockaddr *)&client_addr, &client_addr_length);
  if (bytes <= 0) {
    // sprintf(log_buffer, "Rank %d received message %s [%d] with size %d from
    // %d",
    //         comm->myrank, msg, bytes, comm->mysock, src);
    // logErr(log_buffer);
    return -1;
  }

  bytes = sendto(comm->mysock, &bytes, sizeof(bytes), MSG_DONTWAIT,
                 (struct sockaddr *)&client_addr, client_addr_length);
  if (bytes <= 0) {
    return -1;
  }

  return 0;
}
