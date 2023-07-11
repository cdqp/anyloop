// simplified from <https://github.com/rxi/log.c>
#ifndef AYLP_LOGGING_H_
#define AYLP_LOGGING_H_

#include <stdio.h>
#include <stdarg.h>
#include <stdbool.h>
#include <time.h>


enum { LOG_TRACE, LOG_DEBUG, LOG_INFO, LOG_WARN, LOG_ERROR, LOG_FATAL };

#define log_trace(...) log_impl(LOG_TRACE, __FILE__, __LINE__, __VA_ARGS__)
#define log_debug(...) log_impl(LOG_DEBUG, __FILE__, __LINE__, __VA_ARGS__)
#define log_info(...)  log_impl(LOG_INFO,  __FILE__, __LINE__, __VA_ARGS__)
#define log_warn(...)  log_impl(LOG_WARN,  __FILE__, __LINE__, __VA_ARGS__)
#define log_error(...) log_impl(LOG_ERROR, __FILE__, __LINE__, __VA_ARGS__)
#define log_fatal(...) log_impl(LOG_FATAL, __FILE__, __LINE__, __VA_ARGS__)

void log_init(int level);
int log_get_level(void);
void log_impl(int level, const char *file, int line, const char *fmt, ...);

#endif

