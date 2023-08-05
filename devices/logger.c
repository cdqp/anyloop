#include "anyloop.h"
#include "logging.h"
#include "pretty.h"
#include "logger.h"


int logger_init(struct aylp_device *self)
{
	self->process = &logger_process;
	self->close = &logger_close;
	log_info("logger initialized");
	// set types and units
	self->type_in = AYLP_T_ANY;
	self->units_in = AYLP_U_ANY;
	self->type_out = 0;
	self->units_out = 0;
	return 0;
}


int logger_process(struct aylp_device *self, struct aylp_state *state)
{
	UNUSED(self);
	switch (state->header.type) {
	case AYLP_T_VECTOR:
		log_info("Seeing vector of size %llu:", state->vector->size);
		logn_info("");
		pretty_vector(state->vector);
		break;
	case AYLP_T_MATRIX:
		log_info("Seeing matrix of size %llux%llu:",
			state->matrix->size1, state->matrix->size2
		);
		pretty_matrix(state->matrix);
		break;
	case AYLP_T_MATRIX_UCHAR:
		log_info("Seeing matrix of size %llux%llu:",
			state->matrix_uchar->size1, state->matrix_uchar->size2
		);
		pretty_matrix_uchar(state->matrix_uchar);
		break;
	default:
		log_info("Seeing type %hX but don't know how to print it",
			state->header.type
		);
	}
	return 0;
}


int logger_close(struct aylp_device *self)
{
	UNUSED(self);
	log_info("logger closed");
	return 0;
}

