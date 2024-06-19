#include <gsl/gsl_matrix.h>
#include "anyloop.h"
#include "logging.h"
#include "remove_piston.h"
#include "xalloc.h"


// somehow this doesn't exist in gsl!
static double gsl_matrix_sum_elements(gsl_matrix *m)
{
	double res = 0;
	for (size_t i = 0; i < m->size1; i++) {
		for (size_t j = 0; j < m->size2; j++) {
			res += m->data[i * m->tda + j];
		}
	}
	return res;
}


// like gsl_matrix_add_constant, but doesn't operate in place
static int gsl_matrix_add_constant_copy(
	gsl_matrix *dst, gsl_matrix *src, double x
){
	if (UNLIKELY(dst->size1 != src->size1 || dst->size2 != src->size2)) {
		log_error("BUG: matrices are of different size");
		return -1;
	}
	for (size_t i = 0; i < src->size1; i++) {
		for (size_t j = 0; j < src->size2; j++) {
			dst->data[i * dst->tda + j] = x + src->data[
				i * src->tda + j
			];
		}
	}
	return 0;
}


int remove_piston_init(struct aylp_device *self)
{
	self->device_data = xcalloc(1, sizeof(struct aylp_remove_piston_data));
	self->process = &remove_piston_process;
	self->close = &remove_piston_close;

	// set types and units
	self->type_in = AYLP_T_MATRIX;
	self->units_in = AYLP_U_ANY;
	self->type_out = 0;
	self->units_out = 0;

	return 0;
}


int remove_piston_process(struct aylp_device *self, struct aylp_state *state)
{
	struct aylp_remove_piston_data *data = self->device_data;

	if (UNLIKELY(!data->res_m)) {
		// we have nowhere to put the result; let's allocate it
		data->res_m = gsl_matrix_alloc(
			state->matrix->size1, state->matrix->size2
		);
	} else if (UNLIKELY(data->res_m->size1 != state->matrix->size1
	|| data->res_m->size2 != state->matrix->size2)) {
		// somehow the state matrix changed size >:(
		xfree_type(gsl_matrix, data->res_m);
		data->res_m = gsl_matrix_alloc(
			state->matrix->size1, state->matrix->size2
		);
	}

	double x = -gsl_matrix_sum_elements(state->matrix);
	x /= state->matrix->size1 * state->matrix->size2;
	int err = gsl_matrix_add_constant_copy(data->res_m, state->matrix, x);
	if (err) return -1;

	// update pipeline state
	state->matrix = data->res_m;

	return 0;
}


int remove_piston_close(struct aylp_device *self)
{
	struct aylp_remove_piston_data *data = self->device_data;
	xfree_type(gsl_matrix, data->res_m);
	xfree(data);
	return 0;
}

