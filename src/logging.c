// simplified from <https://github.com/rxi/log.c>
#include "logging.h"

static struct {
	void *udata;
	log_lockfn lock;
	int level;
	bool quiet;
} log_status;

static const char *level_strings[] = {
	"TRACE", "DEBUG", "INFO", "WARN", "ERROR", "FATAL"
};

#ifdef LOG_USE_COLOR
static const char *level_colors[] = {
	"\x1b[94m", "\x1b[36m", "\x1b[32m", "\x1b[33m", "\x1b[31m", "\x1b[35m"
};
#endif

static void lock(void)
{
	if (log_status.lock)
		log_status.lock(true, log_status.udata);
}


static void unlock(void)
{
	if (log_status.lock)
		log_status.lock(false, log_status.udata);
}


const char* log_level_string(int level)
{
	return level_strings[level];
}


void log_set_lock(log_lockfn fn, void *udata)
{
	log_status.lock = fn;
	log_status.udata = udata;
}


void log_set_level(int level)
{
	log_status.level = level;
}


int log_get_level()
{
	return log_status.level;
}


void log_set_quiet(bool enable)
{
	log_status.quiet = enable;
}


static void init_event(struct log_event *ev, void *udata)
{
	if (!ev->time) {
		time_t t = time(0);
		ev->time = localtime(&t);
	}
	ev->udata = udata;
}


void log_log(int level, const char *file, int line, const char *fmt, ...)
{
	struct log_event ev = {
		.fmt   = fmt,
		.file  = file,
		.line  = line,
		.level = level,
	};

	lock();

	if (!log_status.quiet && level >= log_status.level) {
		init_event(&ev, stderr);
		va_start(ev.ap, fmt);
		char buf[16];
		buf[strftime(buf, sizeof(buf), "%H:%M:%S", ev.time)] = '\0';
#ifdef LOG_USE_COLOR
		fprintf(
			ev.udata, "%s %s%-5s\x1b[0m \x1b[90m%s:%d:\x1b[0m ",
			buf, level_colors[ev.level], level_strings[ev.level],
			ev.file, ev.line
		);
#else
		fprintf(
			ev.udata, "%s %-5s %s:%d: ",
			buf, level_strings[ev.level], ev.file, ev.line
		);
#endif
		vfprintf(ev.udata, ev.fmt, ev.ap);
		fprintf(ev.udata, "\n");
		fflush(ev.udata);
		va_end(ev.ap);
	}

	unlock();
}

