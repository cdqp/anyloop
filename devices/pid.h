#ifndef AYLP_DEVICES_PID_H_
#define AYLP_DEVICES_PID_H_

#include <time.h>

#include "anyloop.h"

struct aylp_pid_data {
	// param type in ["vector", "matrix"]
	aylp_type type;
	// accumulated error
	union {
		gsl_vector *acc_v;
		gsl_matrix *acc_m;
	};
	// previous error
	union {
		gsl_vector *pre_v;
		gsl_matrix *pre_m;
	};
	// correction result
	union {
		gsl_vector *res_v;
		gsl_matrix *res_m;
	};
	// previous timestamp
	struct timespec tp;
	// pid params, clamp for maximum correction (by i term and in total)
	double p, i, d, clamp;
};

// initialize pid device
int pid_init(struct aylp_device *self);

// process pid device once per loop
int pid_process(struct aylp_device *self, struct aylp_state *state);

// close pid device when loop exits
int pid_close(struct aylp_device *self);

#endif

