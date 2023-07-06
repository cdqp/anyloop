#include <time.h>
#include <json-c/json.h>
#include "anyloop.h"
#include "logging.h"
#include "delay.h"


int delay_init(struct aylp_device *self)
{
	self->process = &delay_process;
	self->close = &delay_close;
	self->device_data = calloc(1, sizeof(struct timespec)
	);
	struct timespec *ts = self->device_data;
	json_object_object_foreach(self->params, key, val) {
		// parse parameters
		if (key[0] == '_') {
			// keys starting with _ are comments
		} else if (!strcmp(key, "s")) {
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
	// set types
	self->type_in = AYLP_T_ANY | AYLP_U_ANY;
	self->type_out = 0;
	return 0;
}


int delay_process(struct aylp_device *self, struct aylp_state *state)
{
	UNUSED(state);
	nanosleep((struct timespec *)self->device_data, 0);
	return 0;
}


int delay_close(struct aylp_device *self)
{
	free(self->device_data); self->device_data = 0;
	return 0;
}

