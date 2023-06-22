#ifndef _OPENAO_DELAY_H
#define _OPENAO_DELAY_H

#include "openao.h"

// initialize delay device
int delay_init(struct oao_device *self);

// process delay device once per loop
int delay_process(struct oao_device *self, struct oao_state *state);

// close delay device when loop exits
int delay_close(struct oao_device *self);

#endif

