#include <time.h>
#include <json-c/json.h>
#include "anyloop.h"
#include "logging.h"
#include "delay.h"
#include "xalloc.h"


int delay_init(struct aylp_device *self)
{
	self->process = &delay_process;
	self->close = &delay_close;
	self->device_data = xcalloc(1, sizeof(struct timespec));
	struct timespec *ts = self->device_data;
	if (!self->params) {
		log_error("No params object found.");
		return -1;
	}
	json_object_object_foreach(self->params, key, val) {
		// parse parameters
		if (key[0] == '_') {
			// keys starting with _ are comments
		} else if (!strcmp(key, "s")) {
			ts->tv_sec = (time_t)json_object_get_uint64(val);
			log_trace("s = %lu", ts->tv_sec);
		} else if (!strcmp(key, "ns")) {
			ts->tv_nsec = (long)json_object_get_int64(val);
			log_trace("ns = %lu", ts->tv_nsec);
		} else {
			log_warn("Unknown parameter \"%s\"", key);
		}
	}
	// set types and units
	self->type_in = AYLP_T_ANY;
	self->units_in = AYLP_U_ANY;
	self->type_out = 0;
	self->units_out = 0;
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
	xfree(self->device_data);
	return 0;
}

