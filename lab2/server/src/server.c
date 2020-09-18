#include "./../include/Mserver_2.h"

#define PORT 0

extern char *VERSION;
typedef char *message_t;

const int msgCodeIndex = 0;
const int msgUUIDIndex = sizeof(unsigned long int);
const int msgIndex = 2 * sizeof(unsigned long int);

static int shut = 0;
char logBuffer[LOG_BUFFER_SIZE];

static struct sigaction act;
static struct sigaction old;
static sigset_t set;

void sighandler(int s, siginfo_t *info, void *param) {
  pid_t pid = info->si_pid;
  int status = 0;
  sprintf(logBuffer, "Received signal %d from %d", s, pid);
  logSys(logBuffer);
  switch (s) {
    case SIGINT:
      while (!shut) shut = 1;
      break;
    case SIGCHLD:
      wait(&status);
      // Reading exit-code
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
  int pid = -1;
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
  fcntl(socketfd, F_SETFL, FNDELAY | fcntl(socketfd, F_GETFL, 0));
  logSys("Ready for connections...");

  // signal(SIGINT, sighandler);
  // signal(SIGCHLD, sighandler);

  sigemptyset(&set);
  sigaddset(&set, SIGCHLD);
  sigaddset(&set, SIGINT);
  act.sa_sigaction = sighandler;
  act.sa_mask = set;
  act.sa_flags = SA_NOCLDSTOP | SA_RESTART | SA_SIGINFO;

  sigaction(SIGINT, &act, &old);
  sigaction(SIGCHLD, &act, &old);

  while (!shut) {
    clientSocket = acceptTCPConnection(socketfd);

    if (clientSocket < 0) {
      continue;
    }
    if (!(pid = fork())) {
      close(socketfd);
      clientConnection(clientSocket);
      exit(0);
    }

    sprintf(logBuffer, "Create child with pid %d", pid);
    logSys(logBuffer);
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
  int sh = 0;
  int retval = -1;
  int n = 0;
  message_t msg = NULL;
  fd_set descriptors;
  struct timespec timeouts;
  if ((msg = (message_t)malloc(MU * sizeof(char))) == NULL) {
    logFatal("Failed to allocate memory");
  }

  FD_ZERO(&descriptors);
  FD_SET(sock, &descriptors);
  timeouts.tv_sec = 1;
  timeouts.tv_nsec = 0;
  while (!sh) {
    retval = pselect(sock + 1, &descriptors, NULL, NULL, &timeouts, NULL);
    if (retval) { 
      n = recv(sock, msg, MU, 0);
      if (n <= 0) {
        sh = 1;
        break;
      }
      logInfo("Receive");
      logInfo(msg);
      send(sock, msg, strlen(msg), 0);
      logInfo("Sended echo-reply");
    }
  }
  close(sock);
}

int acceptTCPConnection(int server_socket) {
  int connection = -1;
  struct sockaddr_in connection_addr;
  unsigned int len = sizeof(connection_addr);
  int retval = -1;
  fd_set descriptors;
  struct timespec timeouts;
  FD_ZERO(&descriptors);
  FD_SET(server_socket, &descriptors);
  timeouts.tv_sec = 0;
  timeouts.tv_nsec = 1000;
  retval =
      pselect(server_socket + 1, &descriptors, NULL, NULL, &timeouts, NULL);
  if (retval) {
    if ((connection = accept(server_socket, (struct sockaddr *)&connection_addr,
                             &len)) < 0) {
      logErr("Accept failed");

    } else {
      sprintf(logBuffer, "Client conneted, ip: %s",
              inet_ntoa(connection_addr.sin_addr));
      logInfo(logBuffer);
    }
  } else {
    connection = -1;
  }
  return connection;
}