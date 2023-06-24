#ifndef _OPENAO_UDP_SINK_H
#define _OPENAO_UDP_SINK_H

#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/uio.h>
#include "openao.h"

struct oao_udp_sink_data {
	int sock;
	// destination
	struct sockaddr_in dest_sa;
	// iovec for writev so we can write the header and the data in one go
	struct iovec iovecs[2];
};

// initialize udp_sink device
int udp_sink_init(struct oao_device *self);

// process udp_sink device once per loop
int udp_sink_process(struct oao_device *self, struct oao_state *state);

// close udp_sink device when loop exits
int udp_sink_close(struct oao_device *self);

#endif

