#ifndef _OPENAO_UDP_SINK_H
#define _OPENAO_UDP_SINK_H

#include "openao.h"

// initialize udp_sink device
int udp_sink_init(struct oao_device *self);

// process udp_sink device once per loop
int udp_sink_process(struct oao_device *self, struct oao_state *state);

// close udp_sink device when loop exits
int udp_sink_close(struct oao_device *self);

#endif

