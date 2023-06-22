#include <time.h>
#include <json-c/json.h>
#include "openao.h"
#include "logging.h"
#include "delay.h"


int delay_init(struct oao_device *self)
{
	self->process = &delay_process;
	self->close = &delay_close;
	self->device_data = calloc(1, sizeof(struct timespec)
	);
	struct timespec *ts = self->device_data;
	json_object_object_foreach(self->params, key, val) {
		// parse parameters
		if (!strcmp(key, "s")) {
			ts->tv_sec = (time_t) strtoumax(
				json_object_get_string(val), 0, 0
			);
			log_trace("s = %u", ts->tv_sec);
		} else if (!strcmp(key, "ns")) {
			ts->tv_nsec = (long) strtol(
				json_object_get_string(val), 0, 0
			);
			log_trace("ns = %u", ts->tv_nsec);
		} else {
			log_warn("Unknown parameter \"%s\"", key);
		}
	}
	log_info("delay initialized");
	return 0;
}


int delay_process(struct oao_device *self, struct oao_state *state)
{
	nanosleep((struct timespec *)self->device_data, 0);
	log_trace("delay processed");
	return 0;
}


int delay_close(struct oao_device *self)
{
	struct oao_delay_data *data = self->device_data;
	free(data); self->device_data = 0;
	log_info("delay closed");
	return 0;
}

