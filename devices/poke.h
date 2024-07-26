#ifndef AYLP_DEVICES_POKE_H_
#define AYLP_DEVICES_POKE_H_

#include "anyloop.h"
#include <gsl/gsl_matrix.h>
#include <gsl/gsl_vector.h>

struct aylp_poke_data {
	// poke matrix
	gsl_matrix *poke_matrix;
	// current command vector to poke
	gsl_vector *poke;
	// needed for calculating peak-to-peak response vectors
	gsl_vector *tmp;
	// current iteration
	size_t iter;
	// param: total count of actuators
	size_t n_act;
	// param: filename
	const char *filename;
	// pitch between actuators (optional; just for pipeline metadata)
	size_t pitch_y;
	size_t pitch_x;
};

// initialize poke device
int poke_init(struct aylp_device *self);

// process poke device once per loop
int poke_proc(struct aylp_device *self, struct aylp_state *state);

// close poke device when loop exits
int poke_fini(struct aylp_device *self);

#endif

