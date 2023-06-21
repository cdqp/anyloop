#ifndef _OPENAO_VONKARMAN_SCREEN_H
#define _OPENAO_VONKARMAN_SCREEN_H

#include "openao.h"

// initialize vonkarman_screen device
void vonkarman_screen_init(struct oao_device *self);

// process vonkarman_screen device once per loop
void vonkarman_screen_process(struct oao_device *self, struct oao_state *state);

// close vonkarman_screen device when loop exits
void vonkarman_screen_close(struct oao_device *self);

#endif

