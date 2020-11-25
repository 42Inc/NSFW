#include "./../include/Mclient_2.h"
#include "./../include/colorlib.h"

extern char *VERSION;
typedef char *message_t;

int port = 0;
int count = 0;
char logBuffer[LOG_BUFFER_SIZE];
char servAddr_v4[INET_ADDRSTRLEN];
int sh = 0;

message_t defMessage = "echo";
message_t msg = NULL;

void sighandler(int s) {
  sprintf(logBuffer, "Received signal %d", s);
  logSys(logBuffer);
  switch (s) {
    case SIGINT:
      while (!sh) sh = 1;
      break;
    default:
      break;
  }
}

int main(int argc, char **argv) {
  logSys("Started client");
  logSys(logBuffer);
  parseParams(argc, argv);
  setLogLevel(4);
  return startTCPClient();
}

int startTCPClient() {
  struct sockaddr_in servAddr;
  socklen_t servAddrLength = 0;
  struct in_addr servAddr_inaddr;
  struct hostent *host = NULL;
  fd_set descriptors;
  struct timespec timeouts;

  int retval = -1;
  int n = 0;
  int i = 0;
  int socketfd = -1;

  // Requesting socket from system
  if ((socketfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0) {
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

  signal(SIGINT, sighandler);

  if (connect(socketfd, (struct sockaddr *)&servAddr, servAddrLength)) {
    logFatal("Failed to connect");
  }

  timeouts.tv_sec = 1;
  timeouts.tv_nsec = 0;

  while (!sh && i++ < 3) {
    logInfo("Sended echo-request");
    n = sendto(socketfd, msg, MU, MSG_DONTWAIT, NULL, -1);
    if (n <= 0) {
      close(socketfd);
      logFatal("Failed to send");
    }
    sprintf(logBuffer, "Sended %d bytes", n);
    logInfo(logBuffer);

    FD_ZERO(&descriptors);
    FD_SET(socketfd, &descriptors);
    retval = pselect(socketfd + 1, &descriptors, NULL, NULL, &timeouts, NULL);

    if (retval < 0 && errno != EINTR) {
      close(socketfd);
      logFatal("Failed to pselect from socket");
    } else if (retval && errno != EINTR) {
      n = recvfrom(socketfd, msg, MU, MSG_WAITALL, NULL, NULL);
      if (n <= 0) {
        sh = 1;
        break;
      }
      msg[n] = 0;
      sprintf(logBuffer, "Received %d bytes", n);
      logInfo(logBuffer);
      logInfo("Receive");
      logInfo(msg);
    }

    sleep(2);
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

  if ((msg = (message_t)malloc(MU * sizeof(char))) == NULL) {
    logFatal("Failed to allocate memory");
  }
  strcpy(msg, argv[3]);
}
