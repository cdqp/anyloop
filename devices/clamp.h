#ifndef AYLP_DEVICES_CLAMP_H_
#define AYLP_DEVICES_CLAMP_H_

#include <gsl/gsl_matrix.h>
#include "anyloop.h"

// https://en.wikipedia.org/wiki/X_macro
#define FOR_AYLP_CLAMP_TYPES(DO) \
	DO(BLOCK, block) \
	DO(VECTOR, vector) \
	DO(MATRIX, matrix) \
	DO(BLOCK_UCHAR, block_uchar) \
	DO(MATRIX_UCHAR, matrix_uchar)

struct aylp_clamp_data {
	// min and max values to clamp to
	double min;
	double max;
	// the place to put our result
	#define DECLARE_RESULT(_, type) gsl_##type *type;
	FOR_AYLP_CLAMP_TYPES(DECLARE_RESULT)
	#undef DECLARE_RESULT
};

// initialize clamp device
int clamp_init(struct aylp_device *self);

// different process() function for each type
#define DECLARE_PROCESS(_, type) int clamp_process_##type( \
	struct aylp_device *self, struct aylp_state *state \
);
FOR_AYLP_CLAMP_TYPES(DECLARE_PROCESS)
#undef DECLARE_PROCESS

// close clamp device when loop exits
int clamp_close(struct aylp_device *self);

#endif

