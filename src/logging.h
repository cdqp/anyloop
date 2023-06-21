// simplified from <https://github.com/rxi/log.c>
#ifndef _OPENAO_LOGGING_H
#define _OPENAO_LOGGING_H

#define LOG_USE_COLOR

#include <stdio.h>
#include <stdarg.h>
#include <stdbool.h>
#include <time.h>

struct log_event {
	va_list ap;
	const char *fmt;
	const char *file;
	struct tm *time;
	void *udata;
	int line;
	int level;
};

typedef void (*log_logfn)(struct log_event *ev);
typedef void (*log_lockfn)(bool lock, void *udata);

enum { LOG_TRACE, LOG_DEBUG, LOG_INFO, LOG_WARN, LOG_ERROR, LOG_FATAL };

#define log_trace(...) log_log(LOG_TRACE, __FILE__, __LINE__, __VA_ARGS__)
#define log_debug(...) log_log(LOG_DEBUG, __FILE__, __LINE__, __VA_ARGS__)
#define log_info(...)  log_log(LOG_INFO,  __FILE__, __LINE__, __VA_ARGS__)
#define log_warn(...)  log_log(LOG_WARN,  __FILE__, __LINE__, __VA_ARGS__)
#define log_error(...) log_log(LOG_ERROR, __FILE__, __LINE__, __VA_ARGS__)
#define log_fatal(...) log_log(LOG_FATAL, __FILE__, __LINE__, __VA_ARGS__)

const char* log_level_string(int level);
void log_set_lock(log_lockfn fn, void *udata);
void log_set_level(int level);
void log_set_quiet(bool enable);

void log_log(int level, const char *file, int line, const char *fmt, ...);

#endif

