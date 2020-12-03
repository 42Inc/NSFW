#ifndef RMPI_LOGGER_H
#define RMPI_LOGGER_H
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define LOG_BUFFER_SIZE 256

void rmpi_log_info(char* msg);
void rmpi_log_err(char* msg);
void rmpi_log_warn(char* msg);
void rmpi_log_sys(char* msg);
void rmpi_log_fatal(char* msg);
void rmpi_set_log_file(char* file);
void rmpi_set_log_level(int l);
void rmpi_close_log_file(char* file);
static char* rmpi_get_date();

#endif