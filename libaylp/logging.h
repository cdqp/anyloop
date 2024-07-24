// simplified from <https://github.com/rxi/log.c>
#ifndef AYLP_LOGGING_H_
#define AYLP_LOGGING_H_

#include <stdio.h>
#include <stdarg.h>
#include <stdbool.h>
#include <time.h>


enum { LOG_TRACE, LOG_DEBUG, LOG_INFO, LOG_WARN, LOG_ERROR, LOG_FATAL };

// adds trailing newline
#define log_trace(...) \
	log_impl(LOG_TRACE, __FILE_NAME__, __LINE__, 1, __VA_ARGS__)
#define log_debug(...) \
	log_impl(LOG_DEBUG, __FILE_NAME__, __LINE__, 1, __VA_ARGS__)
#define log_info(...) \
	log_impl(LOG_INFO,  __FILE_NAME__, __LINE__, 1, __VA_ARGS__)
#define log_warn(...) \
	log_impl(LOG_WARN,  __FILE_NAME__, __LINE__, 1, __VA_ARGS__)
#define log_error(...) \
	log_impl(LOG_ERROR, __FILE_NAME__, __LINE__, 1, __VA_ARGS__)
#define log_fatal(...) \
	log_impl(LOG_FATAL, __FILE_NAME__, __LINE__, 1, __VA_ARGS__)
// no trailing newline
#define logn_trace(...) \
	log_impl(LOG_TRACE, __FILE_NAME__, __LINE__, 0, __VA_ARGS__)
#define logn_debug(...) \
	log_impl(LOG_DEBUG, __FILE_NAME__, __LINE__, 0, __VA_ARGS__)
#define logn_info(...) \
	log_impl(LOG_INFO,  __FILE_NAME__, __LINE__, 0, __VA_ARGS__)
#define logn_warn(...) \
	log_impl(LOG_WARN,  __FILE_NAME__, __LINE__, 0, __VA_ARGS__)
#define logn_error(...) \
	log_impl(LOG_ERROR, __FILE_NAME__, __LINE__, 0, __VA_ARGS__)
#define logn_fatal(...) \
	log_impl(LOG_FATAL, __FILE_NAME__, __LINE__, 0, __VA_ARGS__)

void log_init(int level);
// returns true on success, false otherwise
bool log_set_level_by_name(char *level_name);
int log_get_level(void);
void log_impl(int level, const char *file, int line, bool newline,
	const char *fmt, ...
);

#endif

