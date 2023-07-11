#ifndef _AYLP_STOP_AFTER_COUNT_H
#define _AYLP_STOP_AFTER_COUNT_H

#include "anyloop.h"

// initialize count device
int stop_after_count_init(struct aylp_device *self);

// process count device once per loop
int stop_after_count_process(
	struct aylp_device *self, struct aylp_state *state
);

// close count device when loop exits
int stop_after_count_close(struct aylp_device *self);

#endif

