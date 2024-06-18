#ifndef AYLP_DEVICES_LOGGER_H_
#define AYLP_DEVICES_LOGGER_H_

#include "anyloop.h"

// initialize logger device
int logger_init(struct aylp_device *self);

// process logger device once per loop
int logger_process(struct aylp_device *self, struct aylp_state *state);

// close logger device when loop exits
int logger_close(struct aylp_device *self);

#endif

