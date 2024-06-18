#ifndef AYLP_DEVICES_CENTER_OF_MASS_H_
#define AYLP_DEVICES_CENTER_OF_MASS_H_

#include "anyloop.h"
#include "thread_pool.h"

struct aylp_center_of_mass_data {
	// param: height of regions/subapertures
	size_t region_height;
	// param: width of regions/subapertures
	size_t region_width;
	// param; set to 1 for no multithreading
	size_t thread_count;
	// array of threads
	pthread_t *threads;
	// array of tasks
	struct aylp_task *tasks;
	// number of tasks
	size_t n_tasks;
	// array of gsl_matrix_uchar inputs for the tasks
	gsl_matrix_uchar *subaps;
	// queue of tasks
	struct aylp_queue queue;

	// center of mass result (contiguous vector)
	gsl_vector *com;
};

// initialize center_of_mass device
int center_of_mass_init(struct aylp_device *self);

// process center_of_mass device once per loop
int center_of_mass_process(struct aylp_device *self, struct aylp_state *state);
// multithreaded version of process function
int center_of_mass_process_threaded(
	struct aylp_device *self, struct aylp_state *state
);

// close center_of_mass device when loop exits
int center_of_mass_close(struct aylp_device *self);
int center_of_mass_close_threaded(struct aylp_device *self);

#endif

