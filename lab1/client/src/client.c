#include <Mclient.h>

extern char *VERSION;
char logBuffer[256];

int port = 0;
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
  char *buffer = NULL;
  int MU = 1024;
  int n = 0;
  if ((socketfd = socket(AF_INET, SOCK_DGRAM, 17)) < 0) {
    logFatal("Socket creation failed");
  }
  memset(&servAddr, 0, sizeof(servAddr));
  if ((host = gethostbyname(servAddr_v4)) == NULL) logFatal("Resolve failed");
  struct in_addr **addr_list = (struct in_addr **)host->h_addr_list;
  for (int i = 0; addr_list[i] != NULL; ++i) {
    servAddr_inaddr = *addr_list[i];
    break;
  }

  servAddr.sin_family = AF_INET;
  servAddr.sin_addr.s_addr = servAddr_inaddr.s_addr;
  servAddr.sin_port = htons(port);

  servAddrLength = sizeof(servAddr);

  if ((buffer = (char *)malloc(sizeof(char) * MU)) == NULL) {
    logFatal("Failed to allocate memory");
  }

  strcpy(buffer, "echo");
  n = sendto(socketfd, (char *)buffer, strlen(buffer), MSG_DONTWAIT,
             (struct sockaddr *)&servAddr, servAddrLength);
  n = recvfrom(socketfd, (char *)buffer, MU, MSG_WAITALL,
               (struct sockaddr *)&servAddr, &servAddrLength);
  logInfo("Receive");
  logInfo(buffer);
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