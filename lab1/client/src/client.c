#include <Mclient.h>

extern char *VERSION;
typedef char *message_t;
const int msgCodeIndex = 0;
const int msgUUIDIndex = sizeof(unsigned long int);
const int msgIndex = 2 * sizeof(unsigned long int);

int port = 0;
int count = 0;
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
  struct sockaddr_in servAddr;
  socklen_t servAddrLength = 0;
  struct in_addr servAddr_inaddr;
  struct hostent *host = NULL;
  struct timespec timeouts;
  message_t msg = NULL;
  fd_set descriptors;
  int socketfd = -1;
  int msgLen = 0;
  int n = 0;
  int i = 0;
  int currentTry = 0;
  int retval = -1;
  unsigned long int msgUUID = 0;
  unsigned long int msgCode = 0;

  // Requesting socket from system
  if ((socketfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0) {
    logFatal("Socket creation failed");
  }

  // Clearing server configuration structure
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

  // Initializing descriptors and timeouts
  FD_ZERO(&descriptors);
  FD_SET(socketfd, &descriptors);
  timeouts.tv_sec = 2;
  timeouts.tv_nsec = 0;

  for (i = 0; i < count; ++i) {
    msgCode = i;
    // Clearing message
    memset(msg, 0, MU);

    // Constructing message
    memcpy(&msg[msgCodeIndex], &msgCode, sizeof(unsigned long int));
    strcpy(&msg[msgIndex], "echo");
    msgLen = msgIndex + strlen(&msg[msgIndex]);
    sprintf(logBuffer, "Send %lu (%d[%d]) from %lu", msgCode, msgLen,
            strlen(&msg[msgIndex]), msgUUID);
    logInfo(logBuffer);
    logInfo(&msg[msgIndex]);
    // Sending message
    n = sendto(socketfd, (message_t)msg, msgLen, MSG_DONTWAIT,
               (struct sockaddr *)&servAddr, servAddrLength);

    // Waiting data
    retval = pselect(socketfd + 1, &descriptors, NULL, NULL, &timeouts, NULL);

    if (retval) {  // Receiving server reply
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
        if (msgCode != i) {
          ++currentTry;
          if (currentTry >= MAXX_TRIES) {
            logFatal("Transmition is impossible");
          }
          sprintf(logBuffer,
                  "Receive ack for wrong packet (Exept %d, Recv %d), try %d", i,
                  msgCode, currentTry);
          logWarn(logBuffer);
          --i;
          continue;
        }
      }
    } else {
      ++currentTry;
      if (currentTry >= MAXX_TRIES) {
        logFatal("Transmition is impossible");
      }
      sprintf(logBuffer, "Dont't receive ack for %d, try %d", msgCode,
              currentTry);
      logWarn(logBuffer);
      --i;
      continue;
    }
    sleep(1);
  }
  logSys("Stopping client...");
  close(socketfd);
  logSys("Done");
  return 0;
}

void parseParams(int argc, char **argv) {
  if (argc < 4) {
    logFatal("Not enouth params");
  }
  strcpy(servAddr_v4, argv[1]);
  port = atoi(argv[2]);
  if (0xFFFF < port && port < 0x0) {
    logFatal("Port out of range");
  }
  count = atoi(argv[3]);
}