#include <Mlogger.h>
static FILE* stream = NULL;
static int level = 2;

void logInfo(char* msg) {
  char* date = getDate();
  if (level > 3) {
    fprintf(stream ? stream : stderr, "%s [INFO]: %s\n", date, msg);
  }
}

void logErr(char* msg) {
  char* date = getDate();
  if (level > 1) {
    fprintf(stream ? stream : stderr, "%s [ERROR]: %s\n", date, msg);
  }
}

void logWarn(char* msg) {
  char* date = getDate();
  if (level > 2) {
    fprintf(stream ? stream : stderr, "%s [WARN]: %s\n", date, msg);
  }
}

void logSys(char* msg) {
  char* date = getDate();
  fprintf(stream ? stream : stderr, "%s [SYS]: %s\n", date, msg);
}

void logFatal(char* msg) {
  char* date = getDate();
  fprintf(stream ? stream : stderr, "%s [FATAL]: %s\n", date, msg);
  exit(1);
}

void setLogFile(char* file) { stream = fopen(file, "a"); }

void setLogLevel(int l) {
  level = l < 0 ? 0 : l;
  char str[32];
  sprintf(str, "Set Log level to %d", l);
  logSys(str);
}

void closeLogFile(char* file) {
  if (stream) {
    fclose(stream);
  }
}

char* getDate() {
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