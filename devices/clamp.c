#include <gsl/gsl_matrix.h>
#include "anyloop.h"
#include "logging.h"
#include "clamp.h"
#include "xalloc.h"


int clamp_init(struct aylp_device *self)
{
	self->device_data = xcalloc(1, sizeof(struct aylp_clamp_data));
	struct aylp_clamp_data *data = self->device_data;

	self->fini = &clamp_fini;

	// default to ±1
	data->min = -1.0;
	data->max = 1.0;

	if (!self->params) {
		log_error("No params object found.");
		return -1;
	}
	json_object_object_foreach(self->params, key, val) {
		// parse parameters
		if (key[0] == '_') {
			// keys starting with _ are comments
		} else if (!strcmp(key, "type")) {
			const char *s = json_object_get_string(val);
			self->type_in = aylp_type_from_string(s);
			log_trace("type = %s (0x%hhX)", s, self->type_in);
		} else if (!strcmp(key, "min")) {
			data->min = json_object_get_double(val);
			log_trace("min = %d", data->min);
		} else if (!strcmp(key, "max")) {
			data->max = json_object_get_double(val);
			log_trace("max = %d", data->max);
		} else {
			log_warn("Unknown parameter \"%s\"", key);
		}
	}
	if (!self->type_in) {
		log_error("You must provide the type parameter.");
		return -1;
	}

	switch (self->type_in) {
	// type-specific setup
	#define INIT_CASE_TYPE(TYPE, type) \
	case AYLP_T_##TYPE: \
		self->proc = &clamp_proc_##type; \
		break;
	FOR_AYLP_CLAMP_TYPES(INIT_CASE_TYPE)
	default:
		log_error("Invalid input type %hhx", self->type_in);
		return -1;
	}

	self->units_in = AYLP_U_ANY;
	self->type_out = AYLP_T_UNCHANGED;
	self->units_out = AYLP_U_UNCHANGED;
	return 0;
}


int clamp_proc_block(struct aylp_device *self, struct aylp_state *state)
{
	struct aylp_clamp_data *data = self->device_data;
	if (UNLIKELY(!data->block)) {
		// we have nowhere to put the result; let's allocate it
		data->block = gsl_block_alloc(state->block->size);
	} else if (UNLIKELY(data->block->size != state->block->size)) {
		// somehow the state block changed size >:(
		xfree_type(gsl_block, data->block);
		data->block = gsl_block_alloc(state->block->size);
	}

	for (size_t i = 0; i < data->block->size; i++) {
		double x = state->block->data[i];
		if (x < data->min) x = data->min;
		else if (data->max < x) x = data->max;
		data->block->data[i] = x;
	}

	state->block = data->block;
	return 0;
}

int clamp_proc_vector(struct aylp_device *self, struct aylp_state *state)
{
	struct aylp_clamp_data *data = self->device_data;
	if (UNLIKELY(!data->vector)) {
		data->vector = gsl_vector_alloc(state->vector->size);
	} else if (UNLIKELY(data->vector->size != state->vector->size)) {
		xfree_type(gsl_vector, data->vector);
		data->vector = gsl_vector_alloc(state->vector->size);
	}

	for (size_t i = 0; i < data->vector->size; i++) {
		double x = state->vector->data[i * state->vector->stride];
		if (x < data->min) x = data->min;
		else if (data->max < x) x = data->max;
		data->vector->data[i * data->vector->stride] = x;
	}

	state->vector = data->vector;
	return 0;
}

int clamp_proc_matrix(struct aylp_device *self, struct aylp_state *state)
{
	struct aylp_clamp_data *data = self->device_data;
	gsl_matrix *src = state->matrix;
	if (UNLIKELY(!data->matrix)) {
		data->matrix = gsl_matrix_alloc(src->size1, src->size2);
	} else if (UNLIKELY(data->matrix->size1 != src->size1
	|| data->matrix->size2 != src->size2)) {
		xfree_type(gsl_matrix, data->matrix);
		data->matrix = gsl_matrix_alloc(src->size1, src->size2);
	}
	gsl_matrix *dst = data->matrix;

	for (size_t i = 0; i < src->size1; i++) {
		for (size_t j = 0; j < src->size2; j++) {
			double x = src->data[i * src->tda + j];
			if (x < data->min) x = data->min;
			else if (data->max < x) x = data->max;
			dst->data[i * dst->tda + j] = x;
		}
	}

	state->matrix = data->matrix;
	return 0;
}

int clamp_proc_block_uchar(
	struct aylp_device *self, struct aylp_state *state
){
	struct aylp_clamp_data *data = self->device_data;
	gsl_block_uchar *src = data->block_uchar;
	if (UNLIKELY(!data->block_uchar)) {
		// we have nowhere to put the result; let's allocate it
		data->block_uchar = gsl_block_uchar_alloc(src->size);
	} else if (UNLIKELY(data->block_uchar->size != src->size)) {
		// somehow the state block_uchar changed size >:(
		xfree_type(gsl_block_uchar, data->block_uchar);
		data->block_uchar = gsl_block_uchar_alloc(
			state->block_uchar->size
		);
	}

	unsigned char min = data->min;
	unsigned char max = data->max;

	for (size_t i = 0; i < data->block_uchar->size; i++) {
		unsigned char x = src->data[i];
		if (x < min) x = min;
		else if (max < x) x = max;
		data->block_uchar->data[i] = x;
	}

	state->block_uchar = data->block_uchar;
	return 0;
}

int clamp_proc_matrix_uchar(
	struct aylp_device *self, struct aylp_state *state
){
	struct aylp_clamp_data *data = self->device_data;
	gsl_matrix_uchar *src = state->matrix_uchar;
	if (UNLIKELY(!data->matrix_uchar)) {
		data->matrix_uchar = gsl_matrix_uchar_alloc(
			src->size1, src->size2
		);
	} else if (UNLIKELY(data->matrix_uchar->size1 != src->size1
	|| data->matrix_uchar->size2 != src->size2)) {
		xfree_type(gsl_matrix_uchar, data->matrix_uchar);
		data->matrix_uchar = gsl_matrix_uchar_alloc(
			src->size1, src->size2
		);
	}
	gsl_matrix_uchar *dst = data->matrix_uchar;

	for (size_t i = 0; i < src->size1; i++) {
		for (size_t j = 0; j < src->size2; j++) {
			unsigned char x = src->data[i * src->tda + j];
			if (x < data->min) x = data->min;
			else if (data->max < x) x = data->max;
			dst->data[i * dst->tda + j] = x;
		}
	}

	state->matrix_uchar = data->matrix_uchar;
	return 0;
}


int clamp_fini(struct aylp_device *self)
{
	struct aylp_clamp_data *data = self->device_data;
	#define FREE_RESULT(_, type) \
		if (data->type) xfree_type(gsl_##type, data->type);
	FOR_AYLP_CLAMP_TYPES(FREE_RESULT)
	xfree(data);
	return 0;
}

