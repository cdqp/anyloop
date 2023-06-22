#ifndef _OPENAO_LOGGER_H
#define _OPENAO_LOGGER_H

#include "openao.h"

// initialize logger device
int logger_init(struct oao_device *self);

// process logger device once per loop
int logger_process(struct oao_device *self, struct oao_state *state);

// close logger device when loop exits
int logger_close(struct oao_device *self);

#endif

