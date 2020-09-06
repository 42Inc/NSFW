#include <Mserver.h>

#define PORT 0

extern char *VERSION;
typedef char *message_t;
const int msgCodeIndex = 0;
const int msgUUIDIndex = sizeof(unsigned long int);
const int msgIndex = 2 * sizeof(unsigned long int);

static int shut = 0;
char logBuffer[LOG_BUFFER_SIZE];

void sighandler(int s) {
  sprintf(logBuffer, "Received signal %d", s);
  logSys(logBuffer);
  switch (s) {
    case SIGINT:
      while (!shut) shut = 1;
      break;
    default:
      break;
  }
}

int main(int argc, char **argv) {
  logSys("Started server");
  sprintf(logBuffer, "Server Vesrion %s", VERSION);
  logSys(logBuffer);
  parseParams(argc, argv);
  setLogLevel(4);
  return startUDPServer();
}

int startUDPServer() {
  struct sockaddr_in servAddr, clAddr;
  socklen_t servAddrLength = 0;
  socklen_t clAddrLength = 0;
  message_t msg = NULL;
  int socketfd = -1;
  int msgLen = 0;
  int n = 0;
  unsigned long int msgUUID = 0;
  unsigned long int msgCode = 0;
  char servAddr_v4[INET_ADDRSTRLEN];

  // Requesting socket from system
  if ((socketfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0) {
    logFatal("Socket creation failed");
  }

  // Clearing server configuration structure
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

  sprintf(logBuffer, "Binded to %s:%d", servAddr_v4,
          ntohs(servAddr.sin_port));
  logSys(logBuffer);
  logSys("Ready for connections...");

  // Allocating memory for message buffer
  if (MU - 2 * sizeof(unsigned long int) <= 0) {
    logFatal("Lower MaxUnit");
  }

  if ((msg = (message_t)malloc(sizeof(char))) == NULL) {
    logFatal("Failed to allocate memory");
  }

  signal(SIGINT, sighandler);

  while (!shut) {
    // Clearing message
    memset(msg, 0, MU);
    // Receiving message
    n = recvfrom(socketfd, (message_t)msg, MU, MSG_WAITALL,
                 (struct sockaddr *)&clAddr, &clAddrLength);
    if (n <= 0) {
      continue;
    }
    // Decoding receive message
    msgLen = n - msgIndex;
    memcpy(&msgCode, &msg[msgCodeIndex], sizeof(unsigned long int));
    memcpy(&msgUUID, &msg[msgUUIDIndex], sizeof(unsigned long int));
    sprintf(logBuffer, "Receive %lu (%d[%d]) from %lu", msgCode, n, msgLen,
            msgUUID);
    logInfo(logBuffer);
    logInfo(&msg[msgIndex]);

    // Replying to client
    sprintf(&msg[msgIndex], "Ack %d [%d]", msgCode, msgUUID);
    msgLen = msgIndex + strlen(&msg[msgIndex]);
    // Constructing reply
    sprintf(logBuffer, "Send %lu (%d[%d]) from %lu", msgCode, n, msgLen,
            msgUUID);
    logInfo(logBuffer);
    logInfo(&msg[msgIndex]);
    n = sendto(socketfd, (message_t)msg, msgLen, MSG_DONTWAIT,
               (struct sockaddr *)&clAddr, clAddrLength);
  }

  logSys("Stopping server...");
  close(socketfd);
  logSys("Done");
  return 0;
}

void parseParams(int argc, char **argv) {}