#include "./../include/rmpi_logger.h"
static FILE* stream = NULL;
static int level = 2;

void rmpi_log_info(char* msg) {
  char* date = rmpi_get_date();
  if (level > 3) {
    fprintf(stream ? stream : stderr, "%s [INFO]: [RMPI] %s\n", date, msg);
  }
}

void rmpi_log_err(char* msg) {
  char* date = rmpi_get_date();
  if (level > 1) {
    fprintf(stream ? stream : stderr, "%s [ERROR]: [RMPI] %s\n", date, msg);
  }
}

void rmpi_log_warn(char* msg) {
  char* date = rmpi_get_date();
  if (level > 2) {
    fprintf(stream ? stream : stderr, "%s [WARN]: [RMPI] %s\n", date, msg);
  }
}

void rmpi_log_sys(char* msg) {
  char* date = rmpi_get_date();
  if (level > 0) {
    fprintf(stream ? stream : stderr, "%s [SYS]: [RMPI] %s\n", date, msg);
  }
}

void rmpi_log_fatal(char* msg) {
  char* date = rmpi_get_date();
  fprintf(stream ? stream : stderr, "%s [FATAL]: [RMPI] %s\n", date, msg);
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

static char* rmpi_get_date() {
  time_t rawtime;
  struct tm* timeinfo;
  char* date = NULL;
  int len = 0;
  time(&rawtime);
  timeinfo = localtime(&rawtime);
  date = asctime(timeinfo);
  len = strlen(date) - 1;
  len = len < 0 ? 0 : len;
  date[len] = '\0';
  return date;
}