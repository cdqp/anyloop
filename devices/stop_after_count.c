#include "anyloop.h"
#include "logging.h"
#include "stop_after_count.h"
#include "xalloc.h"


int stop_after_count_init(struct aylp_device *self)
{
	self->proc = &stop_after_count_proc;
	self->fini = &stop_after_count_fini;
	self->device_data = xcalloc(1, sizeof(size_t));
	size_t *count = self->device_data;
	if (!self->params) {
		log_error("No params object found.");
		return -1;
	}
	json_object_object_foreach(self->params, key, val) {
		if (key[0] == '_') {
			// keys starting with _ are comments
		} else if (!strcmp(key, "count")) {
			*count = json_object_get_uint64(val);
			log_trace("count = %llu", *count);
		} else {
			log_warn("Unknown parameter \"%s\"", key);
		}
	}
	if (!count) {
		log_error("Missing count parameter");
		return -1;
	}
	// set types and units
	self->type_in = AYLP_T_ANY;
	self->units_in = AYLP_U_ANY;
	self->type_out = AYLP_T_UNCHANGED;
	self->units_out = AYLP_U_UNCHANGED;
	return 0;
}


int stop_after_count_proc(struct aylp_device *self, struct aylp_state *state)
{
	size_t *count = (size_t *)self->device_data;
	*count -= 1;
	if (*count == 0) state->header.status |= AYLP_DONE;
	return 0;
}


int stop_after_count_fini(struct aylp_device *self)
{
	UNUSED(self);
	return 0;
}

