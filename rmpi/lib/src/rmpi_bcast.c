#include "./../include/rmpi.h"

static char log_buffer[LOG_BUFFER_SIZE];

int32_t rmpi_bcast(void *msg, int64_t count, size_t type, int64_t src,
                   int64_t tag, comm_t comm) {
  if (!comm) {
    rmpi_log_fatal("Communicator is NULL");
  }

  int64_t i = 0;
  if (src == comm->myrank) {
    sprintf(log_buffer, "Rank %d broadcating message with size %d to all",
            comm->myrank, type * count);
    rmpi_log_info(log_buffer);
    for (i = 0; i < comm->commsize; ++i) {
      if (i == comm->myrank) continue;
      if (rmpi_send(msg, count, type, i, tag, comm)) {
        sprintf(log_buffer, "Send failed [%d -> %d]", comm->myrank, i);
        rmpi_log_fatal(log_buffer);
      }
    }
  } else {
    sprintf(log_buffer, "Rank %d receiving broadcast message size %d from %d",
            comm->myrank, type * count, src);
    rmpi_log_info(log_buffer);
    if (rmpi_recv(msg, count, type, src, tag, comm)) {
      sprintf(log_buffer, "Recv failed [%d -> %d]", src, comm->myrank);
      rmpi_log_fatal(log_buffer);
    }
  }
  return 0;
}
