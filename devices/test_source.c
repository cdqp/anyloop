#include <math.h>

#include "anyloop.h"
#include "logging.h"
#include "test_source.h"
#include "xalloc.h"


enum {KIND_NOISE, KIND_SINE};

int test_source_init(struct aylp_device *self)
{
	self->process = &test_source_process;
	self->close = &test_source_close;
	self->device_data = xcalloc(1, sizeof(struct aylp_test_source_data));
	struct aylp_test_source_data *data = self->device_data;

	// set defaults
	data->freq = 0.1;

	// parse parameters
	if (!self->params) {
		log_error("No params object found.");
		return -1;
	}
	json_object_object_foreach(self->params, key, val) {
		if (key[0] == '_') {
			// keys starting with _ are comments
		} else if (!strcmp(key, "type")) {
			const char *s = json_object_get_string(val);
			if (!strcmp(s, "vector"))
				data->type = AYLP_T_VECTOR;
			else if (!strcmp(s, "matrix"))
				data->type = AYLP_T_MATRIX;
			else log_error("Unrecognized type: %s", s);
			log_trace("type = %s (0x%X)", s, data->type);
		} else if (!strcmp(key, "kind")) {
			const char *s = json_object_get_string(val);
			if (!strcmp(s, "noise")) data->kind = KIND_NOISE;
			else if (!strcmp(s, "sine")) data->kind = KIND_SINE;
			else log_error("Unrecognized kind: %s", s);
			log_trace("kind = %s (0x%X)", s, data->kind);
		} else if (!strcmp(key, "size1")) {
			data->size1 = json_object_get_uint64(val);
			log_trace("size1 = %llu", data->size1);
		} else if (!strcmp(key, "size2")) {
			data->size2 = json_object_get_uint64(val);
			log_trace("size2 = %llu", data->size2);
		} else if (!strcmp(key, "frequency")) {
			data->freq = json_object_get_double(val);
			log_trace("frequency = %E", data->freq);
		} else {
			log_warn("Unknown parameter \"%s\"", key);
		}
	}
	// make sure we didn't miss any params
	if (!data->type || !data->kind) {
		log_error("You must provide valid type and kind params.");
		return -1;
	}

	switch (data->type) {
	case AYLP_T_VECTOR:
		data->vector = gsl_vector_alloc(data->size1);
		break;
	case AYLP_T_MATRIX:
		data->matrix = gsl_matrix_alloc(data->size1, data->size2);
		break;
	}

	// set types and units
	self->type_in = AYLP_T_ANY;
	self->units_in = AYLP_U_ANY;
	self->type_out = data->type;
	self->units_out = 0;
	return 0;
}


int test_source_process(struct aylp_device *self, struct aylp_state *state)
{
	struct aylp_test_source_data *data = self->device_data;

	double val = 0;
	switch (data->kind) {
	case KIND_NOISE:
		log_warn("Noise generation not implemented yet.");	// TODO
		break;
	case KIND_SINE:
		val = sin(data->freq * data->acc);
		break;
	}
	data->acc += 1;

	switch (data->type) {
	case AYLP_T_VECTOR:
		// TODO: move the data->kind switch here instead of set_all
		gsl_vector_set_all(data->vector, val);
		state->vector = data->vector;
		state->header.log_dim.y = data->vector->size;
		state->header.log_dim.x = 1;
		break;
	case AYLP_T_MATRIX:
		gsl_matrix_set_all(data->matrix, val);
		state->matrix = data->matrix;
		state->header.type = self->type_out;
		state->header.log_dim.y = data->matrix->size1;
		state->header.log_dim.x = data->matrix->size2;
		break;
	}
	state->header.type = self->type_out;
	state->header.units = self->units_out;

	return 0;
}


int test_source_close(struct aylp_device *self)
{
	struct aylp_test_source_data *data = self->device_data;
	switch (data->type) {
	case AYLP_T_VECTOR:
		xfree_type(gsl_vector, data->vector);
		break;
	case AYLP_T_MATRIX:
		xfree_type(gsl_matrix, data->matrix);
		break;
	}
	xfree(data);
	return 0;
}

