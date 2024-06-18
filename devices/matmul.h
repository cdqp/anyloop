#ifndef AYLP_DEVICES_MATMUL_H_
#define AYLP_DEVICES_MATMUL_H_

#include "anyloop.h"
#include <gsl/gsl_matrix.h>
#include <gsl/gsl_vector.h>

struct aylp_matmul_data {
	// the matrix we multiply by
	gsl_matrix *mat;
	// the result
	union {
		gsl_matrix *mat_res;
		gsl_vector *vec_res;
	};
};

// initialize matmul device
int matmul_init(struct aylp_device *self);

// matrix-matrix product
int matmul_process_mm(struct aylp_device *self, struct aylp_state *state);

// matrix-vector product
int matmul_process_mv(struct aylp_device *self, struct aylp_state *state);

// close matmul device when loop exits
int matmul_close(struct aylp_device *self);

#endif

