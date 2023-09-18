#include <stdio.h>
#include <json-c/json.h>
#include <gsl/gsl_matrix.h>
#include "anyloop.h"
#include "block.h"
#include "logging.h"
#include "file_sink.h"
#include "xalloc.h"

int file_sink_init(struct aylp_device *self)
{
	self->process = &file_sink_process;
	self->close = &file_sink_close;
	self->device_data = xcalloc(1, sizeof(struct aylp_file_sink_data));
	struct aylp_file_sink_data *data = self->device_data;
	if (!self->params) {
		log_error("No params object found.");
		return -1;
	}
	const char *fn = 0;
	json_object_object_foreach(self->params, key, val) {
		// parse parameters
		if (key[0] == '_') {
			// keys starting with _ are comments
		} else if (!strcmp(key, "filename")) {
			fn = json_object_get_string(val);
			log_trace("filename = %s", fn);
		} else if (!strcmp(key, "flush")) {
			data->flush = json_object_get_boolean(val);
			log_trace("flush = %d", data->flush);
		} else {
			log_warn("Unknown parameter \"%s\"", key);
		}
	}
	if (!fn) {
		log_error("You must provide the filename parameter.");
		return -1;
	}
	data->fp = fopen(fn, "wb");
	if (!data->fp) {
		log_error("Couldn't open file: %s", strerror(errno));
		return -1;
	}
	// set types and units
	self->type_in = AYLP_T_ANY;
	self->units_in = AYLP_U_ANY;
	self->type_out = 0;
	self->units_out = 0;
	return 0;
}


int file_sink_process(struct aylp_device *self, struct aylp_state *state)
{
	struct aylp_file_sink_data *data = self->device_data;
	// write the header
	size_t n;
	n = fwrite(&state->header, 1, sizeof(struct aylp_header), data->fp);
	// write the data
	int needs_free = get_contiguous_bytes(&data->bytes, state);
	if (needs_free < 0) return needs_free;
	n += fwrite(data->bytes.data, 1, data->bytes.size, data->fp);
	size_t n_expect = sizeof(struct aylp_header) + data->bytes.size;
	if (n < n_expect) {
		log_error("Short write: %d of %d", n, n_expect);
		return -1;
	}
	if (needs_free) {
		xfree(data->bytes.data);
	}
	if (data->flush) {
		fflush(data->fp);
	}
	return 0;
}


int file_sink_close(struct aylp_device *self)
{
	struct aylp_file_sink_data *data = self->device_data;
	fflush(data->fp);
	fclose(data->fp);
	xfree(data);
	return 0;
}

