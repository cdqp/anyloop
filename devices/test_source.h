#ifndef AYLP_DEVICES_TEST_SOURCE_H_
#define AYLP_DEVICES_TEST_SOURCE_H_

#include "anyloop.h"

struct aylp_test_source_data {
	// one of ["vector", "matrix", "matrix_uchar"]
	aylp_type type;
	// one of ["constant", "sine"]
	unsigned kind;
	// size of vector or height of matrix
	size_t size1;
	// width of matrix, if applicable
	size_t size2;
	// frequency of sine oscillation, if applicable; units are radians per
	// process() call
	double frequency;
	// amplitude of sine wave, if applicable
	double amplitude;
	// offset of sine wave or value of constant
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

