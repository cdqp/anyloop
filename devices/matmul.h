#ifndef AYLP_DEVICES_MATMUL_H_
#define AYLP_DEVICES_MATMUL_H_

#include "anyloop.h"
#include <gsl/gsl_matrix.h>
#include <gsl/gsl_vector.h>

/** aylp:matmul
 *
 * types and units: `[T_VECTOR|T_MATRIX, U_ANY] -> [T_UNCHANGED, U_UNCHANGED]`
 *
 * This device reads a matrix from a provided AYLP file, and multiplies the
 * vector or matrix that is in the pipeline by the provided matrix. This is a
 * simple but powerful device; depending on the matrix it is given, it could do
 * basis conversions (e.g. Zernike transforms), wavefront reconstruction, etc.
 * The only tricky part is deciding what matrix to feed it!
 *
 * Parameters:
 *   - `filename` (string) (required)
 *     - The filename of the AYLP file to read a matrix from.
 *   - `type` (string) (required)
 *     - "vector" if this device is to do matrix-vector multiplication (and thus
 *       input type is `T_VECTOR`), and "matrix" if this device is to do
 *       matrix-matrix multiplication (for input type `T_MATRIX`).
 */

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

