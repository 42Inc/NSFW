#include <Mserver.h>

#define PORT 0

extern char *VERSION;
char logBuffer[LOG_BUFFER_SIZE];

int main(int argc, char **argv) {
  logSys("Started server");
  sprintf(logBuffer, "Server Vesrion %s", VERSION);
  logSys(logBuffer);
  parseParams(argc, argv);
  setLogLevel(4);
  return startUDPServer();
}

int startUDPServer() {
  int socketfd;
  struct sockaddr_in servAddr, clAddr;
  socklen_t servAddrLength = 0;
  socklen_t clAddrLength = 0;
  char *buffer = NULL;
  int MU = 1024;
  char servAddr_v4[INET_ADDRSTRLEN];
  int n = 0;
  if ((socketfd = socket(AF_INET, SOCK_DGRAM, 17)) < 0) {
    logFatal("Socket creation failed");
  }

  memset(&servAddr, 0, sizeof(servAddr));

  // Setting the server structure
  servAddr.sin_family = AF_INET;
  servAddr.sin_addr.s_addr = INADDR_ANY;
  servAddr.sin_port = htons(PORT);

  // Getting sizeof of structures
  servAddrLength = sizeof(servAddr);
  clAddrLength = sizeof(clAddr);

  // Binding socket
  if (bind(socketfd, (const struct sockaddr *)&servAddr, servAddrLength) < 0) {
    logFatal("Bind failed");
  }

  // Getting socket params
  if (getsockname(socketfd, (struct sockaddr *)&servAddr, &servAddrLength)) {
    logFatal("Getsockname failed");
  }

  // Transformation address to readable form
  inet_ntop(AF_INET, &servAddr.sin_addr, servAddr_v4, sizeof(servAddr_v4));

  sprintf(logBuffer, "Binded to %s:%02d", servAddr_v4,
          ntohs(servAddr.sin_port));
  logSys(logBuffer);
  logSys("Ready for connections...");

  // Allocating memory for message buffer
  if ((buffer = (char *)malloc(sizeof(char) * MU)) == NULL) {
    logFatal("Failed to allocate memory");
  }

  // Receiving message
  n = recvfrom(socketfd, (char *)buffer, MU, MSG_WAITALL,
               (struct sockaddr *)&clAddr, &clAddrLength);
  buffer[n] = '\0';
  logInfo("Receive");
  logInfo(buffer);

  // Replying to client
  n = sendto(socketfd, (char *)buffer, n, MSG_DONTWAIT,
             (struct sockaddr *)&clAddr, clAddrLength);

  logSys("Stopping server...");
  close(socketfd);
  logSys("Done");
  return 0;
}

void parseParams(int argc, char **argv) {}