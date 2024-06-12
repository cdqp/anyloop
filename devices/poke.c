#include <gsl/gsl_matrix.h>
#include <gsl/gsl_vector.h>
#include <json-c/json.h>
#include "anyloop.h"
#include "block.h"
#include "logging.h"
#include "poke.h"
#include "xalloc.h"


int poke_init(struct aylp_device *self)
{
	self->process = &poke_process;
	self->close = &poke_close;
	self->device_data = xcalloc(1, sizeof(struct aylp_poke_data));
	struct aylp_poke_data *data = self->device_data;

	// parse the params json into our data struct
	if (!self->params) {
		log_error("No params object found.");
		return -1;
	}
	json_object_object_foreach(self->params, key, val) {
		if (key[0] == '_') {
			// keys starting with _ are comments
		} else if (!strcmp(key, "n_act")) {
			data->n_act = json_object_get_uint64(val);
			log_trace("n_act = %llu", data->n_act);
		} else if (!strcmp(key, "filename")) {
			data->filename = json_object_get_string(val);
			log_trace("filename = %s", data->filename);
		} else {
			log_warn("Unknown parameter \"%s\"", key);
		}
	}

	// make sure we didn't miss any params
	if (!data->n_act || !data->filename) {
		log_error("You must provide the n_act and filename params.");
		return -1;
	}

	// allocate command vector
	data->poke = gsl_vector_alloc(data->n_act);

	// set types and units
	self->type_in = AYLP_T_VECTOR;
	self->units_in = AYLP_U_MINMAX;
	self->type_out = AYLP_T_VECTOR;
	self->units_out = AYLP_U_MINMAX;

	return 0;
}


int poke_process(struct aylp_device *self, struct aylp_state *state)
{
	struct aylp_poke_data *data = self->device_data;
	int err;

	if (data->iter == 0) {
		// first iteration; determine error vector size and alloc
		data->poke_matrix = gsl_matrix_alloc(
			state->vector->size,
			data->n_act
		);
		data->tmp = gsl_vector_alloc(state->vector->size);
	} else if (data->iter & 1) {
		// odd iteration; set tmp to this and continue
		err = gsl_vector_memcpy(data->tmp, state->vector);
		if (err) {
			log_error("Error in copying response vector: ",
				gsl_strerror(err)
			);
		}
	} else {
		// even iteration; we have gotten both response vectors
		// (max - min) / 2
		err = gsl_vector_sub(data->tmp, state->vector);
		if (err) {
			log_error("Error in subtracting response vectors: ",
				gsl_strerror(err)
			);
		}
		err = gsl_vector_scale(data->tmp, 0.5);
		if (err) {
			log_error("Error in scaling response vector: ",
				gsl_strerror(err)
			);
		}
		// set poke matrix column
		err = gsl_matrix_set_col(
			data->poke_matrix, (data->iter >> 1) - 1, data->tmp
		);
		if (err) {
			log_error("Error in setting poke matrix column: ",
				gsl_strerror(err)
			);
		}
	}

	if (data->iter >> 1 >= data->n_act) {
		FILE *fp = fopen(data->filename, "wb");
		// temporarily update header and state to poke matrix
		state->matrix = data->poke_matrix;
		state->header.type = AYLP_T_MATRIX;
		state->header.units = AYLP_U_MINMAX;
		state->header.log_dim.y = data->poke_matrix->size1;
		state->header.log_dim.x = data->poke_matrix->size2;
		state->header.pitch.y = data->pitch_y;
		state->header.pitch.x = data->pitch_x;
		// write header to file
		size_t n;
		n = fwrite(&state->header, 1, sizeof(struct aylp_header), fp);
		// write poke matrix to file
		gsl_block_uchar bytes;
		int needs_free = get_contiguous_bytes(&bytes, state);
		if (needs_free < 0) return needs_free;
		n += fwrite(bytes.data, 1, bytes.size, fp);
		size_t n_expect = sizeof(struct aylp_header) + bytes.size;
		if (n < n_expect) {
			log_error("Short write: %d of %d", n, n_expect);
			return -1;
		}
		if (needs_free) {
			xfree(bytes.data);
		}
		// we're done here
		state->header.status = AYLP_DONE;
	} else {
		data->iter += 1;
		// zero out the command vector
		gsl_vector_set_zero(data->poke);
		if (data->iter & 1) {
			// odd means poke to max
			gsl_vector_set(data->poke, data->iter >> 1, 1.0);
		} else {
			// even means poke to min
			gsl_vector_set(data->poke, (data->iter >> 1) - 1, -1.0);
		}
	}

	// update pipeline state
	state->vector = data->poke;
	// housekeeping on the header
	state->header.type = AYLP_T_VECTOR;
	state->header.units = AYLP_U_MINMAX;
	state->header.log_dim.y = data->n_act;
	state->header.log_dim.x = 1;
	state->header.pitch.y = data->pitch_y;
	state->header.pitch.x = data->pitch_x;
	return 0;
}


int poke_close(struct aylp_device *self)
{
	struct aylp_poke_data *data = self->device_data;
	xfree_type(gsl_matrix, data->poke_matrix);
	xfree_type(gsl_vector, data->poke);
	xfree_type(gsl_vector, data->tmp);
	xfree(data);
	return 0;
}

