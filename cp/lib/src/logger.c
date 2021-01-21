#include "./../include/rmpi_logger.h"
static FILE* stream = NULL;
static int level = 2;

void rmpi_log_info(char* msg) {
  if (level > 3) {
    fprintf(stream ? stream : stderr, "[INFO]: [RMPI] %s\n", msg);
  }
}

void rmpi_log_err(char* msg) {
  if (level > 1) {
    fprintf(stream ? stream : stderr, "[ERROR]: [RMPI] %s\n", msg);
  }
}

void rmpi_log_warn(char* msg) {
  if (level > 2) {
    fprintf(stream ? stream : stderr, "[WARN]: [RMPI] %s\n", msg);
  }
}

void rmpi_log_sys(char* msg) {
  if (level > 0) {
    fprintf(stream ? stream : stderr, "[SYS]: [RMPI] %s\n", msg);
  }
}

void rmpi_log_fatal(char* msg) {
  fprintf(stream ? stream : stderr, "[FATAL]: [RMPI] %s\n", msg);
  exit(1);
}

void rmpi_set_log_file(char* file) { stream = fopen(file, "a"); }

void rmpi_set_log_level(int l) {
  level = l < 0 ? 0 : l;
  char str[32];
  sprintf(str, "Set Log level to %d", l);
  rmpi_log_sys(str);
}

void rmpi_close_log_file(char* file) {
  if (stream) {
    fclose(stream);
  }
}
