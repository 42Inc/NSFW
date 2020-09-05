#include <Mclient.h>

extern char *VERSION;
typedef char *message_t;
const int msgCodeIndex = 0;
const int msgUUIDIndex = sizeof(unsigned long int);
const int msgIndex = 2 * sizeof(unsigned long int);

int port = 0;
char logBuffer[LOG_BUFFER_SIZE];
char servAddr_v4[INET_ADDRSTRLEN];

int main(int argc, char **argv) {
  logSys("Started client");
  sprintf(logBuffer, "Client Vesrion %s", VERSION);
  logSys(logBuffer);
  parseParams(argc, argv);
  setLogLevel(4);
  return startUDPClient();
}

int startUDPClient() {
  int socketfd;
  struct sockaddr_in servAddr;
  socklen_t servAddrLength = 0;
  struct in_addr servAddr_inaddr;
  struct hostent *host;
  message_t msg;
  int msgLen = 0;
  unsigned long int msgCode = 0;
  unsigned long int msgUUID = 0;
  char *buffer = NULL;
  int n = 0;

  if ((socketfd = socket(AF_INET, SOCK_DGRAM, 17)) < 0) {
    logFatal("Socket creation failed");
  }

  memset(&servAddr, 0, sizeof(servAddr));

  // Getting addr structure from string
  if ((host = gethostbyname(servAddr_v4)) == NULL) logFatal("Resolve failed");
  struct in_addr **addr_list = (struct in_addr **)host->h_addr_list;
  for (int i = 0; addr_list[i] != NULL; ++i) {
    servAddr_inaddr = *addr_list[i];
    break;
  }

  // Setting the server structure for connect to him
  servAddr.sin_family = AF_INET;
  servAddr.sin_addr.s_addr = servAddr_inaddr.s_addr;
  servAddr.sin_port = htons(port);

  // Getting sizeof of structures
  servAddrLength = sizeof(servAddr);

  // Allocating memory for message buffer
  if (MU - 2 * sizeof(unsigned long int) <= 0) {
    logFatal("Lower MaxUnit");
  }

  if ((msg = (message_t)malloc(sizeof(char))) == NULL) {
    logFatal("Failed to allocate memory");
  }

  memset(msg, 0, MU);

  // Constructing message
  strcpy(&msg[msgIndex], "echo");
  msgLen = msgIndex + strlen(&msg[msgIndex]);
  sprintf(logBuffer, "Send %lu (%d[%d]) from %lu", msgCode, msgLen, strlen(&msg[msgIndex]), msgUUID);
  logInfo(logBuffer);
  logInfo(&msg[msgIndex]);
  // Sending message
  n = sendto(socketfd, (message_t)msg, msgLen, MSG_DONTWAIT,
             (struct sockaddr *)&servAddr, servAddrLength);
  // Receiving server reply
  n = recvfrom(socketfd, (message_t)msg, MU, MSG_WAITALL,
               (struct sockaddr *)&servAddr, &servAddrLength);
  if (n <= 0) {
    logErr("Received message length <= 0");
  } else {
    // Decoding receive message
    msgLen = n - msgIndex;
    memcpy(&msgCode, &msg[msgCodeIndex], sizeof(unsigned long int));
    memcpy(&msgUUID, &msg[msgUUIDIndex], sizeof(unsigned long int));
    sprintf(logBuffer, "Receive %lu (%d[%d]) from %lu", msgCode, n, msgLen,
            msgUUID);
    logInfo(logBuffer);
    logInfo(&msg[msgIndex]);
  }
  logSys("Stopping client...");
  close(socketfd);
  logSys("Done");
  return 0;
}

void parseParams(int argc, char **argv) {
  if (argc < 3) {
    logFatal("Not enouth params");
  }
  strcpy(servAddr_v4, argv[1]);
  port = atoi(argv[2]);
  if (0xFFFF < port && port < 0x0) {
    logFatal("Port out of range");
  }
}