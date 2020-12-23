#ifndef M_LOGGER_H
#define M_LOGGER_H
#include "./colorlib.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define LOG_BUFFER_SIZE 256

void logInfo(char* msg);
void logErr(char* msg);
void logWarn(char* msg);
void logSys(char* msg);
void logFatal(char* msg);
void setLogFile(char* file);
void setLogLevel(int l);
void closeLogFile(char* file);
char* getDate();

#endif
