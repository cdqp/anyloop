#ifndef AYLP_DEVICES_UDP_SINK_H_
#define AYLP_DEVICES_UDP_SINK_H_

#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/uio.h>
#include "anyloop.h"

/** aylp:udp_sink
 *
 * types and units: `[T_ANY, U_ANY] -> [T_UNCHANGED, U_UNCHANGED]`
 *
 * This device writes the current pipeline state to a UDP port as an AYLP file.
 * See [filetype.md](../doc/filetype.md) for documentation on the AYLP file
 * format.
 *
 * Parameters:
 *   - `ip` (string) (required)
 *     - The IP address to send the data to.
 *   - `port` (string) (required)
 *     - The port to send the data to.
 */

struct aylp_udp_sink_data {
	int sock;
	// destination
	struct sockaddr_in dest_sa;
	// iovec for writev so we can write the header and the data in one go
	struct iovec iovecs[2];
	// gsl_block_uchar that we will copy pointer to data to
	gsl_block_uchar bytes;
};

// initialize udp_sink device
int udp_sink_init(struct aylp_device *self);

// process udp_sink device once per loop
int udp_sink_process(struct aylp_device *self, struct aylp_state *state);

// close udp_sink device when loop exits
int udp_sink_close(struct aylp_device *self);

#endif

