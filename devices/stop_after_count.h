#ifndef AYLP_DEVICES_STOP_AFTER_COUNT_H_
#define AYLP_DEVICES_STOP_AFTER_COUNT_H_

#include "anyloop.h"

// initialize count device
int stop_after_count_init(struct aylp_device *self);

// process count device once per loop
int stop_after_count_proc(struct aylp_device *self, struct aylp_state *state);

// close count device when loop exits
int stop_after_count_fini(struct aylp_device *self);

#endif

