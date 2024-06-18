#ifndef AYLP_DEVICES_STOP_AFTER_COUNT_H_
#define AYLP_DEVICES_STOP_AFTER_COUNT_H_

#include "anyloop.h"

/** aylp:stop_after_count
 *
 * types and units: `[T_ANY, U_ANY] -> [T_UNCHANGED, U_UNCHANGED]`
 *
 * This device stops the loop after a certain number of iterations.
 *
 * Parameters:
 *   - `count` (integer) (required)
 *     - The number of iterations to stop after (e.g. count = 1 means every
 *       device in the loop will run once).
 */


// initialize count device
int stop_after_count_init(struct aylp_device *self);

// process count device once per loop
int stop_after_count_process(
	struct aylp_device *self, struct aylp_state *state
);

// close count device when loop exits
int stop_after_count_close(struct aylp_device *self);

#endif

