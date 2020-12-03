#include "./../include/main.h"
#include "./../lib/include/rmpi.h"
char log_buffer[LOG_BUFFER_SIZE];

int main(int argc, char **argv) {
  setLogLevel(4);
  rmpi_init(&argc, &argv, 4);
  if (argc < 2) {
    logFatal("Not enouth args");
  }
  int src = atoi(argv[1]);
  int64_t rank = rmpi_get_rank(RMPI_COMM_WORLD);
  int64_t commsize = rmpi_get_commsize(RMPI_COMM_WORLD);

  char *message = malloc(10 * sizeof(char));
  if (!message) {
    logFatal("Cannot allocate memory");
  }
  if (rank == src) {
    strcpy(message, argv[1]);
  }
  rmpi_bcast(message, 10, RMPI_TYPE_CHAR, src, 0, RMPI_COMM_WORLD);
  
  sprintf(log_buffer, "Rank %d/%d [%s]", rank, commsize, message);
  logInfo(log_buffer);
  logSys("Sleep");
  sleep(10);
  rmpi_finilize();
  return 0;
}
