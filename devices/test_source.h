#ifndef AYLP_DEVICES_TEST_SOURCE_H_
#define AYLP_DEVICES_TEST_SOURCE_H_

#include "anyloop.h"

struct aylp_test_source_data {
	// param: one of ["vector", "matrix", "matrix_uchar"]
	aylp_type type;
	// param: one of ["constant", "sine"]
	unsigned kind;
	// to put in pipeline
	union {
		gsl_vector *vector;
		gsl_matrix *matrix;
		gsl_matrix_uchar *matrix_uchar;
	};
	// param: size of vector or height of matrix
	size_t size1;
	// param: width of matrix, if needed
	size_t size2;
	// param: frequency of sine oscillation in radians per process() call
	double frequency;
	// param: amplitude of sine wave; note that output will be clipped to ±1
	double amplitude;
	// param: offset of sine wave or value of constant
	double offset;
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

