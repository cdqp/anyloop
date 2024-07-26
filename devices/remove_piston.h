#ifndef AYLP_DEVICES_REMOVE_PISTON_H_
#define AYLP_DEVICES_REMOVE_PISTON_H_

#include <gsl/gsl_matrix.h>
#include "anyloop.h"

struct aylp_remove_piston_data {
	// the result
	gsl_matrix *res_m;
};

// initialize remove_piston device
int remove_piston_init(struct aylp_device *self);

// process remove_piston once per loop
int remove_piston_proc(struct aylp_device *self, struct aylp_state *state);

// close remove_piston device when loop exits
int remove_piston_fini(struct aylp_device *self);

#endif

