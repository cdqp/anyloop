#include "logging.h"

#include <stdio.h>
#include <time.h>
#include <string.h>

static log_status_t *log_status;

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


void log_init(log_status_t *status)
{
	log_status = status;
}

bool log_set_level_by_name(char *level_name)
{
	const size_t num_levels = sizeof(level_strings)/sizeof(*level_strings);
	for (size_t i=0; i<num_levels; i++) {
		if (strcmp(level_name, level_strings[i]) == 0) {
			log_status->level = i;
			return true;
		}

	}

	log_fatal("Log level must be one of the following values:");
	for (size_t i=0; i<num_levels; i++) {
		log_fatal(" - %s", level_strings[i]);
	}
	return false;
}

int log_get_level(void)
{
	return log_status->level;
}

void log_impl(int level, const char *file, int line, const char *fmt, ...)
{
	if (level < log_status->level)
		return;

	// perform color check
	const char *level_prefix = "";
	const char *file_prefix = "";
	const char *color_reset = "";

	if (log_status->use_color) {
		level_prefix = level_colors[level];
		file_prefix = "\x1b[37m";
		color_reset = "\x1b[0m";
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
		stderr, "%s %s%-5s%s %s:%d:%s ",
		time_str, level_prefix, level_strings[level], file_prefix,
		file, line, color_reset
	);

	va_list args;
	va_start(args, fmt);
	vfprintf(stderr, fmt, args);
	putc('\n', stderr);
	fflush(stderr);
	va_end(args);
}

