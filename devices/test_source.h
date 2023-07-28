#ifndef AYLP_DEVICES_TEST_SOURCE_H_
#define AYLP_DEVICES_TEST_SOURCE_H_

#include "anyloop.h"

struct aylp_test_source_data {
	// param type in ["vector", "matrix"]
	aylp_type type;
	// param kind in ["noise", "sine"]
	unsigned kind;
	// to put in pipeline
	union {
		gsl_vector *vector;
		gsl_matrix *matrix;
	};
	// size of vector or height of matrix
	size_t size1;
	// width of matrix, if needed
	size_t size2;
	// frequency of sine oscillation in radians per process() call
	double freq;
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

