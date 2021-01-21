#include "./../include/Mlogger.h"
static FILE* stream = NULL;
static int level = 2;

void logInfo(char* msg) {
  if (level > 3) {
    fprintf(stream ? stream : stderr, "[INFO]: %s\n", msg);
  }
}

void logErr(char* msg) {
  if (level > 1) {
    fprintf(stream ? stream : stderr, "[ERROR]: %s\n", msg);
  }
}

void logWarn(char* msg) {
  if (level > 2) {
    fprintf(stream ? stream : stderr, "[WARN]: %s\n", msg);
  }
}

void logSys(char* msg) {
  if (level > 0) {
    fprintf(stream ? stream : stderr, "[SYS]: %s\n", msg);
  }
}

void logFatal(char* msg) {
  fprintf(stream ? stream : stderr, "[FATAL]: %s\n", msg);
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
