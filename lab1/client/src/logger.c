#include "./../include/Mlogger_1.h"
static FILE* stream = NULL;
static int level = 2;

void logInfo(char* msg) {

  if (level > 3) {
    fprintf(stream ? stream : stderr, "%s[INFO]%s: %s\n", BLUE, RESET, msg);
  }
}

void logErr(char* msg) {
  if (level > 1) {
    fprintf(stream ? stream : stderr, "%s[ERROR]%s: %s\n", RED, RESET, msg);
  }
}

void logWarn(char* msg) {
  if (level > 2) {
    fprintf(stream ? stream : stderr, "%s[WARN]%s: %s\n", YELLOW, RESET, msg);
  }
}

void logSys(char* msg) {
  fprintf(stream ? stream : stderr, "%s[SYS]%s: %s\n", GREEN, RESET, msg);
}

void logFatal(char* msg) {
  fprintf(stream ? stream : stderr, "%s[FATAL]%s: %s\n", RED, RESET, msg);
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
