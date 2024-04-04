#ifndef AYLP_DEVICES_TEST_SOURCE_H_
#define AYLP_DEVICES_TEST_SOURCE_H_

#include "anyloop.h"

struct aylp_test_source_data {
	/** Param: one of ["vector", "matrix", "matrix_uchar"]. */
	aylp_type type;

	/** Param: one of ["constant", "sine", "matrix_uchar"]. */
	unsigned kind;

	/** Param: size of vector or height of matrix. */
	size_t size1;

	/** Param: width of matrix, if applicable. */
	size_t size2;

	/** Param: frequency of sine oscillation, if applicable.
	* Units are radians per process() call */
	double frequency;

	/** Param: amplitude of sine wave, if applicable.
	* Note that for "vector" and "matrix" types, this device outputs units
	* of AYLP_U_MINMAX; the output will be clipped to ±1.0. For the
	* "matrix_uchar" type, the output is instead clipped between 0:255. */
	double amplitude;

	/** Param: offset of sine wave or value of constant. */
	double offset;

	// to put in pipeline
	union {
		gsl_vector *vector;
		gsl_matrix *matrix;
		gsl_matrix_uchar *matrix_uchar;
	};
	// accumulator
	size_t acc;
};

// initialize test_source device
int test_source_init(struct aylp_device *self);

// process test_source device once per loop
int test_source_process(struct aylp_device *self, struct aylp_state *state);

// close test_source device when loop exits
int test_source_close(struct aylp_device *self);

#endif

