#ifndef AYLP_DEVICES_CENTER_OF_MASS_H_
#define AYLP_DEVICES_CENTER_OF_MASS_H_

#include "anyloop.h"
#include "thread_pool.h"

/** aylp:center_of_mass
 *
 * types and units: `[T_MATRIX_UCHAR, U_ANY] -> [T_VECTOR, U_MINMAX]`
 *
 * This device breaks up an image into one or more regions, and calculates the
 * center-of-mass coordinate of that image. For example, this device might be
 * used with only one region to determine the center-of-mass coordinates of a
 * beam on a camera, which can then be used to control a tip-tilt mirror to
 * recenter said beam. This device is also used with many regions for getting
 * error signals from a wavefront sensor.
 *
 * An example configuration for a wavefront sensor with 8x8-pixel subapertures:
 *
 * ```json
 * {
 *   "uri": "anyloop:center_of_mass",
 *     "params": {
 *       "region_height": 8,
 *       "region_width": 8,
 *       "thread_count": 1
 *     }
 * }
 * ```
 *
 * Pipeline data is replaced with a vector of interleaved center-of-mass y and x
 * coordinates (a vector of length 2N, where N is the number of regions of
 * interest). For example, if the input has four regions of interest, the output
 * will be [y1,x1,y2,x2,y3,x3,y4,x4] where each y,x is from -1 to 1, where 0
 * means perfectly centered in the region of interest. It is assumed that the
 * input is written in order of increasing x coordinate, then increasing y
 * coordinate.
 *
 * Parameters:
 *   - `region_height` (integer) (required)
 *     - Height of each region to find the center of mass of. The image will be
 *       split up into regions of this height, from the top going down. Excess
 *       data will be ignored. Set this to 0 to set the region height to the
 *       logical height of the whole image.
 *   - `region_width` (integer) (required)
 *     - Width of each region to find the center of mass of. The image will be
 *       split up into regions of this width, from left to right. Excess data
 *       will be ignored. Set this to 0 to set the region width to the logical
 *       height of the whole image.
 *   - `thread_count` (integer) (optional)
 *     - Number of threads to use for the calculation. Set this to 1 (default)
 *       for no multithreading.
 */

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

