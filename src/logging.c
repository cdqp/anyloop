// simplified from <https://github.com/rxi/log.c>
#include "logging.h"

static struct {
	void *udata;
	log_lockfn lock;
	int level;
	bool quiet;
} L;

static const char *level_strings[] = {
	"TRACE", "DEBUG", "INFO", "WARN", "ERROR", "FATAL"
};

#ifdef LOG_USE_COLOR
static const char *level_colors[] = {
	"\x1b[94m", "\x1b[36m", "\x1b[32m", "\x1b[33m", "\x1b[31m", "\x1b[35m"
};
#endif

static void lock(void)   {
	if (L.lock) { L.lock(true, L.udata); }
}


static void unlock(void) {
	if (L.lock) { L.lock(false, L.udata); }
}


const char* log_level_string(int level) {
	return level_strings[level];
}


void log_set_lock(log_lockfn fn, void *udata) {
	L.lock = fn;
	L.udata = udata;
}


void log_set_level(int level) {
	L.level = level;
}


void log_set_quiet(bool enable) {
	L.quiet = enable;
}

static void init_event(struct log_event *ev, void *udata) {
	if (!ev->time) {
		time_t t = time(NULL);
		ev->time = localtime(&t);
	}
	ev->udata = udata;
}


void log_log(int level, const char *file, int line, const char *fmt, ...) {
	struct log_event ev = {
		.fmt   = fmt,
		.file  = file,
		.line  = line,
		.level = level,
	};

	lock();

	if (!L.quiet && level >= L.level) {
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

