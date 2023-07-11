#include <stdio.h>
#include <json-c/json.h>
#include <gsl/gsl_matrix.h>
#include "anyloop.h"
#include "block.h"
#include "logging.h"
#include "file_sink.h"


int file_sink_init(struct aylp_device *self)
{
	self->process = &file_sink_process;
	self->close = &file_sink_close;
	self->device_data = (struct aylp_file_sink_data*)calloc(
		1, sizeof(struct aylp_file_sink_data)
	);
	struct aylp_file_sink_data *data = self->device_data;
	if (!data) {
		log_error("Couldn't allocate device data: %s", strerror(errno));
		return -1;
	}
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
	// set type
	self->type_in = AYLP_T_ANY | AYLP_U_ANY;
	self->type_out = 0;
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
		free(data->bytes.data); data->bytes.data = 0;
	}
	return 0;
}


int file_sink_close(struct aylp_device *self)
{
	json_object_object_foreach(self->params, key, _) {
		json_object_object_del(self->params, key);
	}
	free(self->params); self->params = 0;
	struct aylp_file_sink_data *data = self->device_data;
	fflush(data->fp);
	fclose(data->fp);
	free(data); self->device_data = 0;
	return 0;
}

