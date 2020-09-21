#include "./../include/Mclient_2.h"

extern char *VERSION;
typedef char *message_t;
const int msgCodeIndex = 0;
const int msgUUIDIndex = sizeof(unsigned long int);
const int msgIndex = 2 * sizeof(unsigned long int);

int port = 0;
int count = 0;
char logBuffer[LOG_BUFFER_SIZE];
char servAddr_v4[INET_ADDRSTRLEN];
int sh = 0;

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
  sprintf(logBuffer, "Client Vesrion %s", VERSION);
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
  message_t msg = NULL;
  struct timespec timeouts;
  int retval = -1;
  int n = 0;
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

  if ((msg = (message_t)malloc(MU * sizeof(char))) == NULL) {
    logFatal("Failed to allocate memory");
  }

  strcpy(msg, "echo");

  signal(SIGINT, sighandler);


  if (connect(socketfd, (struct sockaddr *)&servAddr, servAddrLength)) {
    logFatal("Failed to connect");
  }

  FD_ZERO(&descriptors);
  FD_SET(socketfd, &descriptors);
  timeouts.tv_sec = 1;
  timeouts.tv_nsec = 0;

  while (!sh) {
    logInfo("Sended echo-request");
    n = send(socketfd, msg, strlen(msg), 0);
    if (n <= 0) {
      close(socketfd);
      logFatal("Failed to send");
    }
    retval = pselect(socketfd + 1, &descriptors, NULL, NULL, &timeouts, NULL);
    if (retval) {  
      n = recv(socketfd, msg, MU, 0);
      if (n <= 0) {
        sh = 1;
        break;
      }
      msg[n] = 0;
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
  if (argc < 3) {
    logFatal("Not enouth params");
  }
  strcpy(servAddr_v4, argv[1]);
  port = atoi(argv[2]);
  if (0xFFFF < port && port < 0x0) {
    logFatal("Port out of range");
  }
}