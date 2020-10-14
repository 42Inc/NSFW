#include "./../include/Mserver.h"

#define PORT 0

extern char *VERSION;
typedef char *message_t;

static int shut = 0;
static int childs = 0;
pthread_mutex_t childsLocker = PTHREAD_MUTEX_INITIALIZER;
char logBuffer[LOG_BUFFER_SIZE];
message_t msg = NULL;

static struct sigaction act;
static struct sigaction old;
static sigset_t set;

const int msgCodeIndex = 0;
const int msgUUIDIndex = sizeof(unsigned long int);
const int msgIndex = 2 * sizeof(unsigned long int);

typedef struct params {
  int sock;
} params_t;

void sighandler(int s, siginfo_t *info, void *param) {
  pid_t pid = info->si_pid;
  // int status = 0;
  sprintf(logBuffer, "Received signal %d from %d", s, pid);
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
  return startMultiServer();
}

int startMultiServer() {
  struct sockaddr_in servAddrTCP;
  struct sockaddr_in servAddrUDP;
  socklen_t servAddrUDPLength = 0;
  socklen_t servAddrTCPLength = 0;
  int pid = -1;
  int socketfdTCP = -1;
  int socketfdUDP = -1;
  int res = -1;
  char servAddrTCP_v4[INET_ADDRSTRLEN];
  char servAddrUDP_v4[INET_ADDRSTRLEN];

  // Requesting TCP socket from system
  if ((socketfdTCP = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0) {
    logFatal("TCP socket creation failed");
  }

  // Getting sizeof of structures
  servAddrTCPLength = sizeof(servAddrTCP);

  // Clearing server configuration structure
  memset(&servAddrTCP, 0, servAddrTCPLength);

  // Setting the server structure
  servAddrTCP.sin_family = AF_INET;
  servAddrTCP.sin_addr.s_addr = INADDR_ANY;
  servAddrTCP.sin_port = htons(PORT);

  // Binding socket
  if (bind(socketfdTCP, (const struct sockaddr *)&servAddrTCP,
           servAddrTCPLength) < 0) {
    logFatal("Bind failed");
  }

  // Getting socket params
  if (getsockname(socketfdTCP, (struct sockaddr *)&servAddrTCP,
                  &servAddrTCPLength)) {
    logFatal("Getsockname failed");
  }

  // Transformation address to readable form
  inet_ntop(AF_INET, &servAddrTCP.sin_addr, servAddrTCP_v4,
            sizeof(servAddrTCP_v4));

  sprintf(logBuffer, "TCP server binded to %s:%02d", servAddrTCP_v4,
          ntohs(servAddrTCP.sin_port));
  logSys(logBuffer);

  if (listen(socketfdTCP, 10) < 0) {
    logSys("Listen failed");
  }
  fcntl(socketfdTCP, F_SETFL, FNDELAY | fcntl(socketfdTCP, F_GETFL, 0));
  logSys("TCP server ready for connections...");

  // Requesting UDP socket from system
  if ((socketfdUDP = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0) {
    logFatal("UDP Socket creation failed");
  }

  // Getting sizeof of structures
  servAddrUDPLength = sizeof(servAddrUDP);

  // Clearing server configuration structure
  memset(&servAddrUDP, 0, servAddrUDPLength);

  // Setting the server structure
  servAddrUDP.sin_family = AF_INET;
  servAddrUDP.sin_addr.s_addr = INADDR_ANY;
  servAddrUDP.sin_port = servAddrTCP.sin_port;

  // Binding socket
  if (bind(socketfdUDP, (const struct sockaddr *)&servAddrUDP,
           servAddrUDPLength) < 0) {
    logFatal("Bind failed");
  }

  // Getting socket params
  if (getsockname(socketfdUDP, (struct sockaddr *)&servAddrUDP,
                  &servAddrUDPLength)) {
    logFatal("Getsockname failed");
  }

  // Transformation address to readable form
  inet_ntop(AF_INET, &servAddrUDP.sin_addr, servAddrUDP_v4,
            sizeof(servAddrUDP_v4));

  sprintf(logBuffer, "UDP server binded to %s:%d", servAddrUDP_v4,
          ntohs(servAddrUDP.sin_port));
  logSys(logBuffer);
  logSys("UDP server ready for connections...");

  sigemptyset(&set);
  sigaddset(&set, SIGINT);
  act.sa_sigaction = sighandler;
  act.sa_mask = set;
  act.sa_flags = SA_NOCLDSTOP | SA_RESTART | SA_SIGINFO;

  sigaction(SIGINT, &act, &old);

  // Allocating memory for message buffer
  if (MU - 2 * sizeof(unsigned long int) <= 0) {
    logFatal("Lower MaxUnit");
  }

  if ((msg = (message_t)malloc(MU * sizeof(char))) == NULL) {
    logFatal("Failed to allocate memory");
  }

  while (!shut) {
    res = acceptConnection(socketfdTCP, socketfdUDP);
  }

  logSys("Stopping server...");
  close(socketfdTCP);
  close(socketfdUDP);
  logSys("Done");
  return 0;
}

void parseParams(int argc, char **argv) {}

void *clientTCPConnection(void *args) {
  int sh = 0;
  int retval = -1;
  int n = 0;
  int tries = 0;
  message_t msg = NULL;
  fd_set descriptors;
  struct timespec timeouts;
  int sock = ((params_t *)args)->sock;
  if ((msg = (message_t)malloc(MU * sizeof(char))) == NULL) {
    logFatal("Failed to allocate memory");
  }

  sprintf(logBuffer, "Processing socket %d", sock);
  logInfo(logBuffer);
  timeouts.tv_sec = 1;
  timeouts.tv_nsec = 0;

  while (!sh) {
    FD_ZERO(&descriptors);
    FD_SET(sock, &descriptors);
    retval = pselect(sock + 1, &descriptors, NULL, NULL, &timeouts, NULL);

    if (retval < 0 && errno != EINTR) {
      close(sock);
      logFatal("Failed to pselect from socket");
    } else if (retval && errno != EINTR) {
      n = recvfrom(sock, msg, MU, MSG_WAITALL, NULL, NULL);
      if (n <= 0) {
        sh = 1;
        break;
      }
      msg[n] = 0;
      sprintf(logBuffer, "TCP Received %d bytes", n);
      logInfo(logBuffer);
      logInfo("TCP Receive");
      logInfo(msg);
      tries = 0;
      n = sendto(sock, msg, MU, MSG_DONTWAIT, NULL, -1);
      if (n <= 0) {
        close(sock);
        logFatal("TCP Failed to send");
      }
      logInfo("TCP Sended echo-reply");
      sprintf(logBuffer, "TCP Sended %d bytes", n);
      logInfo(logBuffer);
    }
    ++tries;
    if (tries > MAX_TRIES) {
      logSys("Tries exceeded");
      while (!sh) sh = 1;
    }
  }

  logSys("Stopping TCP connection...");
  close(sock);
  logSys("TCP Connection Terminated");
  return NULL;
}

int acceptConnection(int sTCP, int sUDP) {
  int connection = -1;
  struct sockaddr_in connection_addr;
  unsigned int len = sizeof(connection_addr);
  struct sockaddr_in clAddrUDP;
  socklen_t clAddrUDPLength = 0;
  int retval = -1;
  int n = 0;
  int max_s = MAX(sTCP, sUDP) + 1;
  unsigned long int msgCode = 0;
  fd_set descriptors;
  struct timespec timeouts;
  pthread_t pid;

  // Getting sizeof of structures
  clAddrUDPLength = sizeof(clAddrUDP);

  FD_ZERO(&descriptors);
  FD_SET(sTCP, &descriptors);
  FD_SET(sUDP, &descriptors);
  timeouts.tv_sec = 0;
  timeouts.tv_nsec = 1000;
  retval = pselect(max_s, &descriptors, NULL, NULL, &timeouts, NULL);
  if (retval < 0 && errno != EINTR) {
    close(sTCP);
    close(sUDP);
    logFatal("Failed to pselect from socket");
  }
  if (retval) {
    if (FD_ISSET(sTCP, &descriptors)) {
      if ((connection =
               accept(sTCP, (struct sockaddr *)&connection_addr, &len)) < 0) {
      } else {
        sprintf(logBuffer, "TCP client conneted, ip: %s",
                inet_ntoa(connection_addr.sin_addr));
        logErr("Accept failed");
        params_t p;
        p.sock = connection;
        logInfo(logBuffer);
        pthread_create(&pid, NULL, clientTCPConnection, &p);
        pthread_detach(pid);
      }
    } else if (FD_ISSET(sUDP, &descriptors)) {
      n = recvfrom(sUDP, (message_t)msg, MU, MSG_WAITALL,
                   (struct sockaddr *)&clAddrUDP, &clAddrUDPLength);

      memcpy(&msgCode, &msg[msgCodeIndex], sizeof(unsigned long int));
      if (n <= 0) {
        logErr("UDP Received message length <= 0");
      } else {
        sprintf(logBuffer, "UDP Received %d bytes", n);
        logInfo(logBuffer);
        logInfo("UDP Receive");
        logInfo(&msg[msgIndex]);
      }

      if (msgCode != 3)
        n = sendto(sUDP, (message_t)msg, MU, MSG_DONTWAIT,
                   (struct sockaddr *)&clAddrUDP, clAddrUDPLength);
    }
  } else {
    connection = -1;
  }
  return connection;
}