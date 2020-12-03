#include "./../include/rmpi.h"

static char log_buffer[LOG_BUFFER_SIZE];

int32_t rmpi_send(void *msg, int64_t count, size_t type, int64_t dst,
                  int64_t tag, comm_t comm) {
  if (!comm) {
    rmpi_log_fatal("Communicator is NULL");
  }
  sprintf(log_buffer, "Rank %d sending message with size %d to %d",
          comm->myrank, type * count, dst);
  rmpi_log_info(log_buffer);

  struct sockaddr_in client_addr;
  socklen_t client_addr_length = sizeof(client_addr);
  memset(&client_addr, 0, client_addr_length);
  int64_t index = rmpi_get_index_by_rank(dst, comm);
  rmpi_create_client_socket(dst, comm);

  int32_t current_try = 0;
  int64_t bytes = 0;
  size_t size = type * count;
  while (current_try < RMPI_SEND_TRIES) {
    bytes = sendto(comm->hosts[index].sock, msg, size, MSG_DONTWAIT,
                   (struct sockaddr *)&comm->hosts[index].server_addr,
                   comm->hosts[index].server_addr_length);
    if (bytes <= 0) {
      return -1;
    }

    bytes = recvfrom(comm->hosts[index].sock, &bytes, sizeof(bytes), MSG_WAITALL,
                     (struct sockaddr *)&client_addr, &client_addr_length);
    if (bytes <= 0) {
      return -1;
    }
    break;
    ++current_try;
  }
  return 0;
}
