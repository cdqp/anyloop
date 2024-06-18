#ifndef AYLP_DEVICES_DELAY_H_
#define AYLP_DEVICES_DELAY_H_

#include "anyloop.h"

/** aylp:delay
 *
 * types and units: `[T_ANY, U_ANY] -> [T_UNCHANGED, U_UNCHANGED]`
 *
 * This device pauses execution of the loop for a certain period of time.
 *
 * Parameters:
 *   - `s` (integer) (optional)
 *     - Number of seconds to delay (default 0).
 *   - `ns` (integer) (optional)
 *     - Number of nanoseconds to delay; must be less than 1E9 (default 0).
 */

// initialize delay device
int delay_init(struct aylp_device *self);

// process delay device once per loop
int delay_process(struct aylp_device *self, struct aylp_state *state);

// close delay device when loop exits
int delay_close(struct aylp_device *self);

#endif

