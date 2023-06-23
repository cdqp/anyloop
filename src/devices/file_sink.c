#include <stdio.h>
#include <json-c/json.h>
#include <gsl/gsl_matrix.h>
#include "openao.h"
#include "logging.h"
#include "file_sink.h"


int file_sink_init(struct oao_device *self)
{
	self->process = &file_sink_process;
	self->close = &file_sink_close;
	self->device_data = (struct oao_file_sink_data*)calloc(
		1, sizeof(struct oao_file_sink_data)
	);
	struct oao_file_sink_data *data = self->device_data;
	json_object_object_foreach(self->params, key, val) {
		// parse parameters
		if (key[0] == '_') {
			// keys starting with _ are comments
		} else if (!strcmp(key, "filename")) {
			data->filename = strdup(json_object_get_string(val));
			log_trace("filename = %s", data->filename);
		} else {
			log_warn("Unknown parameter \"%s\"", key);
		}
	}
	if (!data->filename) {
		log_error("You must provide the filename parameter.");
		return 1;
	}
	return 0;
}


int file_sink_process(struct oao_device *self, struct oao_state *state)
{
	// TODO: error checking
	struct oao_file_sink_data *data = self->device_data;
	FILE *fp = fopen(data->filename, "wb");
	gsl_block_fwrite(fp, &state->block);
	fflush(fp);
	fclose(fp);
	return 0;
}


int file_sink_close(struct oao_device *self)
{
	json_object_object_foreach(self->params, key, _) {
		json_object_object_del(self->params, key);
	}
	free(self->params); self->params = 0;
	struct oao_file_sink_data *data = self->device_data;
	free(data->filename); data->filename = 0;
	free(data); self->device_data = 0;
	return 0;
}

