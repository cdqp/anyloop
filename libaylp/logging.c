// simplified from <https://github.com/rxi/log.c>
#include "logging.h"

#include <stdio.h>
#include <time.h>
#include <unistd.h>
#include <string.h>

static struct {
	int level;
	bool use_color;
} log_status;

static const char *level_strings[] = {
	[LOG_TRACE] = "TRACE",
	[LOG_DEBUG] = "DEBUG",
	[LOG_INFO]  = "INFO",
	[LOG_WARN]  = "WARN",
	[LOG_ERROR] = "ERROR",
	[LOG_FATAL] = "FATAL"
};

static const char *level_colors[] = {
	[LOG_TRACE] = "\x1b[94m",
	[LOG_DEBUG] = "\x1b[36m",
	[LOG_INFO]  = "\x1b[32m",
	[LOG_WARN]  = "\x1b[33m",
	[LOG_ERROR] = "\x1b[31m",
	[LOG_FATAL] = "\x1b[35m"
};


void log_init(int level)
{
	log_status.level = level;
	log_status.use_color = isatty(STDERR_FILENO);
}

int log_get_level(void)
{
	return log_status.level;
}

void log_impl(int level, const char *file, int line, const char *fmt, ...)
{
	if (level < log_status.level)
		return;

	// perform color check
	const char *color_prefix = "";
	const char *color_suffix = "";

	if (log_status.use_color) {
		color_prefix = level_colors[level];
		color_suffix = "\x1b[0m";
	}

	// format timestamp string
	char time_str[16];
	time_t current_time = time(NULL);
	struct tm current_tm;
	bool localtime_fail = localtime_r(&current_time, &current_tm) == NULL;
	size_t time_len = strftime(
		time_str, sizeof(time_str), "%H:%M:%S", &current_tm
	);
	time_str[time_len] = '\0';

	if (current_time == (time_t) -1 || localtime_fail || time_len == 0) {
		strncpy(time_str, "TIME_ERROR", sizeof(time_str));
	}

	// display log message
	fprintf(
		stderr, "%s %s%-5s%s %s:%d: ",
		time_str, color_prefix, level_strings[level], color_suffix,
		file, line
	);

	va_list args;
	va_start(args, fmt);
	vfprintf(stderr, fmt, args);
	putc('\n', stderr);
	fflush(stderr);
	va_end(args);
}

