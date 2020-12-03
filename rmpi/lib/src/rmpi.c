#include "./../include/rmpi.h"

static uint16_t port = 0;
static comm_t COMM_WORLD = NULL;
static char log_buffer[LOG_BUFFER_SIZE];

struct netconfig *nconf;
struct netbuf *svcaddr;

extern char *VERSION;

void rmpi_init(int *argc, char ***argv, uint8_t debug) {
  rmpi_set_log_level(debug);
  sprintf(log_buffer, "Vesrion %s", VERSION);
  rmpi_log_sys(log_buffer);

  rmpi_parse_params(argc, argv);
  rmpi_create_socket(COMM_WORLD);
  rmpi_register();
  rmpi_get_ports(COMM_WORLD);
  rmpi_print_comm(COMM_WORLD);
  sleep(RMPI_INIT_SLEEP);
}

void rmpi_finilize() {
  close(COMM_WORLD->mysock);
  int64_t i = 0;
  for (i = 0; i < COMM_WORLD->commsize; ++i) {
    if (COMM_WORLD->hosts[i].sock > 0) {
      close(COMM_WORLD->hosts[i].sock);
    }
  }
  rmpi_log_sys("Finalize");
}

comm_t rmpi_get_comm_world() { return COMM_WORLD; }

void rmpi_print_comm(comm_t comm) {
  int64_t i = 0;
  if (!comm) {
    rmpi_log_fatal("Communicator is NULL");
  }
  sprintf(log_buffer, "Commsize: %d", comm->commsize);
  rmpi_log_info(log_buffer);
  sprintf(log_buffer, "My rank: %d", comm->myrank);
  rmpi_log_info(log_buffer);
  sprintf(log_buffer, "My sock: %d", comm->mysock);
  rmpi_log_info(log_buffer);
  rmpi_log_info("Initialized hosts:");
  for (i = 0; i < comm->commsize; ++i) {
    sprintf(log_buffer, "[%d/%d] %s:%d", comm->hosts[i].rank + 1,
            comm->commsize, comm->hosts[i].host.addr, comm->hosts[i].host.port);
    rmpi_log_info(log_buffer);
  }
}

void rmpi_parse_params(int *argc, char ***argv) {
  if (*argc < 3) {
    rmpi_log_fatal("Not enouth args");
  }
  COMM_WORLD = (comm_t)malloc(sizeof(comm_t));
  if (!COMM_WORLD) {
    rmpi_log_fatal("Cannot create world communicator");
  }
  int64_t rank = atol((*argv)[1]);
  int64_t comm = atol((*argv)[2]);
  int64_t i = 0;
  if (*argc < comm + 1) {
    rmpi_log_fatal("Not enouth hosts");
  }
  if (0 > rank || rank >= comm) {
    rmpi_log_fatal("Incorrect rank");
  }

  sprintf(log_buffer, "Initializing %d processes", comm);
  rmpi_log_sys(log_buffer);
  COMM_WORLD->commsize = comm;
  COMM_WORLD->myrank = rank;
  COMM_WORLD->hosts =
      (hosts_list_t *)malloc(COMM_WORLD->commsize * sizeof(hosts_list_t));
  for (i = 0; i < comm; ++i) {
    COMM_WORLD->hosts[i].rank = i;
    COMM_WORLD->hosts[i].host.addr =
        (char *)malloc(INET_ADDRSTRLEN * sizeof(char));
    strcpy(COMM_WORLD->hosts[i].host.addr, (*argv)[3 + i]);
    COMM_WORLD->hosts[i].host.port = 0;
    COMM_WORLD->hosts[i].sock = -1;
  }
  *argc = *argc - (2 + comm);
  if (*argc > 0) {
    char *prog = (*argv)[0];
    *argv = &((*argv)[2 + comm]);
    (*argv)[0] = prog;
  }
}

void rmpi_create_socket(comm_t comm) {
  COMM_WORLD->mysock = rmpi_create_server_socket();
}

int64_t rmpi_get_index_by_rank(int64_t rank, comm_t comm) {
  int64_t i = 0;
  int64_t index = -1;
  for (i = 0; i < comm->commsize; ++i) {
    if (comm->hosts[i].rank == rank) {
      index = i;
      break;
    }
  }
  return index;
}

int32_t rmpi_create_client_socket(int64_t rank, comm_t comm) {
  int32_t socketfd = -1;
  struct in_addr server_addr_inaddr;
  struct hostent *host = NULL;
  int64_t index = rmpi_get_index_by_rank(rank, comm);
  if (index < 0) {
    return -1;
  }
  // Requesting socket from system
  if ((socketfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0) {
    rmpi_log_fatal("Socket creation failed");
  }
  comm->hosts[index].server_addr_length =
      sizeof(comm->hosts[index].server_addr);
  // Clearing server configuration structure
  memset(&comm->hosts[index].server_addr, 0,
         comm->hosts[index].server_addr_length);

  // Getting addr structure from string
  if ((host = gethostbyname(comm->hosts[index].host.addr)) == NULL)
    rmpi_log_fatal("Resolve failed");
  struct in_addr **addr_list = (struct in_addr **)host->h_addr_list;
  for (int i = 0; addr_list[i] != NULL; ++i) {
    server_addr_inaddr = *addr_list[i];
    break;
  }

  // Setting the server structure for connect to him
  comm->hosts[index].server_addr.sin_family = AF_INET;
  comm->hosts[index].server_addr.sin_addr.s_addr = server_addr_inaddr.s_addr;
  comm->hosts[index].server_addr.sin_port = htons(comm->hosts[index].host.port);
  comm->hosts[index].sock = socketfd;
  return socketfd;
}

int32_t rmpi_create_server_socket() {
  struct sockaddr_in serv_addr;
  char serv_addr_v4[INET_ADDRSTRLEN];
  socklen_t serv_addr_length = 0;
  int32_t socketfd = -1;

  // Requesting socket from system
  if ((socketfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0) {
    rmpi_log_fatal("Socket creation failed");
  }

  // Getting sizeof of structures
  serv_addr_length = sizeof(serv_addr);
  // Clearing server configuration structure
  memset(&serv_addr, 0, serv_addr_length);

  // Setting the server structure
  serv_addr.sin_family = AF_INET;
  serv_addr.sin_addr.s_addr = INADDR_ANY;
  serv_addr.sin_port = htons(0);

  // Binding socket
  if (bind(socketfd, (const struct sockaddr *)&serv_addr, serv_addr_length) <
      0) {
    rmpi_log_fatal("Bind failed");
  }

  // Getting socket params
  if (getsockname(socketfd, (struct sockaddr *)&serv_addr, &serv_addr_length)) {
    rmpi_log_fatal("Getsockname failed");
  }

  // Transformation address to readable form
  inet_ntop(AF_INET, &serv_addr.sin_addr, serv_addr_v4, sizeof(serv_addr_v4));
  port = ntohs(serv_addr.sin_port);
  sprintf(log_buffer, "Binded to %s:%d", serv_addr_v4, port);
  rmpi_log_sys(log_buffer);
  rmpi_log_sys("Ready for connections...");
  return socketfd;
}

uint16_t rmpi_server(char *p) { return port; }

void *start_svc() {
  svc_run();
  return NULL;
}

char **get_port(indata_p) int *indata_p;
{
  static char *res;
  static char p[6] = "";
  sprintf(p, "%d", port);
  res = p;
  return &res;
}

void rmpi_get_ports(comm_t comm) {
  if (!comm) {
    rmpi_log_fatal("Communicator is NULL");
  }
  int64_t i = 0;
  for (i = 0; i < comm->commsize; ++i) {
    comm->hosts[i].host.port =
        rmpi_get_port(comm->hosts[i].host.addr, comm->hosts[i].rank);
  }
}

int32_t rmpi_get_port(char *host, int64_t rank) {
  int32_t i = 0;
  int stat;
  int arg = 0;
  char *answer = malloc(6 * sizeof(char));
  for (i = 0; i < RMTTRIESLIMIT; ++i) {
    sprintf(log_buffer, "Call rpc rank %d to %d on %s", COMM_WORLD->myrank,
            rank, host);
    rmpi_log_sys(log_buffer);
    if (stat = callrpc(host, RMTPROGNUM, rank, RMTPROGPROC, xdr_int, &arg,
                       xdr_wrapstring, &answer) != 0) {
      clnt_perrno(stat);
      usleep(RMTTRIESSLEEP);
    } else {
      break;
    }
  }
  if (i >= RMTTRIESLIMIT) {
    sprintf(log_buffer, "Failed call rpc, rank %d to %d on %s",
            COMM_WORLD->myrank, rank, host);
    rmpi_log_fatal(log_buffer);
  }
  return atoi(answer);
}

int32_t rmpi_register() {
  registerrpc(RMTPROGNUM, COMM_WORLD->myrank, RMTPROGPROC, get_port, xdr_int,
              xdr_wrapstring);
  pthread_t tid = 0;
  pthread_create(&tid, NULL, start_svc, NULL);
  pthread_detach(tid);
  sprintf(log_buffer, "Register rpc, rank %d", COMM_WORLD->myrank);
  rmpi_log_sys(log_buffer);
  return 0;
}

int64_t rmpi_get_rank(comm_t comm) { return comm->myrank; }

int64_t rmpi_get_commsize(comm_t comm) { return comm->commsize; }
