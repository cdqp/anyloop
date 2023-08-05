#ifndef AYLP_DEVICES_CENTER_OF_MASS_H_
#define AYLP_DEVICES_CENTER_OF_MASS_H_

#include "anyloop.h"
#include "thread_pool.h"

struct aylp_center_of_mass_data {
	/** Param: height of each region to find the center of mass of.
	* The image will be split up into regions of this height, from the top
	* going down. Excess data will be ignored. Set this to 0 to set the
	* region height to the logical height of the whole image. */
	size_t region_height;

	/** Param: width of each region to find the center of mass of.
	* The image will be split up into regions of this width, from left to
	* right. Excess data will be ignored. Set this to 0 to set the region
	* height to the logical height of the whole image. */
	size_t region_width;

	/** Param: number of threads to use for the calculation.
	* Set this to 1 for no multithreading. */
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

/** Process center_of_mass device once per loop.
* Will replace pipeline data with a vector of interleaved center-of-mass y and x
* coordinates (a vector of length 2N, where N is the number of regions of
* interest). For example, if the input has four regions of interest, the output
* will be [y1,x1,y2,x2,y3,x3,y4,x4] where each y,x is from -1 to 1, where 0
* means perfectly centered in the region of interest. It is assumed that the
* input is of type AYLP_T_MATRIX_UCHAR, and is written in order of increasing x
* coordinate, then increasing y coordinate. */
int center_of_mass_process(struct aylp_device *self, struct aylp_state *state);
/** Multithreaded version of process function. */
int center_of_mass_process_threaded(
	struct aylp_device *self, struct aylp_state *state
);

// close center_of_mass device when loop exits
int center_of_mass_close(struct aylp_device *self);
int center_of_mass_close_threaded(struct aylp_device *self);

#endif

