#include <Mserver_2.h>

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
    case SIGCHLD:
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
  return startTCPServer();
}

int startTCPServer() {
  struct sockaddr_in servAddr, clAddr;
  socklen_t servAddrLength = 0;
  socklen_t clAddrLength = 0;
  int socketfd = -1;
  int clientSocket = -1;
  char servAddr_v4[INET_ADDRSTRLEN];

  // Requesting socket from system
  if ((socketfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0) {
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

  sprintf(logBuffer, "Binded to %s:%02d", servAddr_v4,
          ntohs(servAddr.sin_port));
  logSys(logBuffer);

  if (listen(socketfd, 10) < 0) {
    logSys("Listen failed");
  }

  logSys("Ready for connections...");

  signal(SIGINT, sighandler);
  signal(SIGCHLD, sighandler);

  while (!shut) {
    clientSocket = acceptTCPConnection(socketfd);

    if (clientSocket < 0) {
      continue;
    }
    if (!fork()) {
      close(socketfd);
      clientConnection(clientSocket);
      exit(0);
    }
    // Parent doesn.t need this
    close(clientSocket);
  }

  logSys("Stopping server...");
  close(socketfd);
  logSys("Done");
  return 0;
}

void parseParams(int argc, char **argv) {}

void clientConnection(int sock) {
  message_t msg = NULL;
  int sh = 0;
  int n = 0;
  fd_set descriptors;
  struct timespec timeouts;
  int retval = -1;
  if ((msg = (message_t)malloc(MU * sizeof(char))) == NULL) {
    logFatal("Failed to allocate memory");
  }

  FD_ZERO(&descriptors);
  FD_SET(sock, &descriptors);
  timeouts.tv_sec = 1;
  timeouts.tv_nsec = 0;
  n = send(sock, msg, strlen(msg), 0);
  while (!sh) {
    retval = pselect(sock + 1, &descriptors, NULL, NULL, &timeouts, NULL);
    if (retval) {  // Receiving server reply
      // n = recv(sock);
    }
  }
  close(sock);
}

int acceptTCPConnection(int server_socket) {
  int connection = -1;
  struct sockaddr_in connection_addr;
  unsigned int len = sizeof(connection_addr);
  if ((connection = accept(server_socket, (struct sockaddr *)&connection_addr,
                           &len)) < 0) {
    logErr("Accept failed");
  }
  sprintf(logBuffer, "Client conneted, ip: %s",
          inet_ntoa(connection_addr.sin_addr));
  logInfo(logBuffer);
  return connection;
}