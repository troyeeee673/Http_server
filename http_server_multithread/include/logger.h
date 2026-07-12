#ifndef __LOGGER_H
#define __LOGGER_H

#include <stdio.h>

void log_init(const char *file_path);
void log_info(const char *format, ...);
void log_error(const char *format, ...);
void log_close(void);

#endif