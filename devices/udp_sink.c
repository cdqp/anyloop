#include <errno.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <json-c/json.h>
#include <netinet/in.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/uio.h>
#include "anyloop.h"
#include "block.h"
#include "logging.h"
#include "udp_sink.h"
#include "xalloc.h"


int udp_sink_init(struct aylp_device *self)
{
	self->process = &udp_sink_process;
	self->close = &udp_sink_close;
	self->device_data = xcalloc(1, sizeof(struct aylp_udp_sink_data));
	struct aylp_udp_sink_data *data = self->device_data;

	data->sock = socket(AF_INET, SOCK_DGRAM, 0);
	if (data->sock == -1) {
		log_error("Couldn't initialize socket: %s", strerror(errno));
		return -1;
	}
	memset(&(data->dest_sa), 0, sizeof(data->dest_sa));
	data->dest_sa.sin_family = AF_INET;
	int got_params = 0;	// use this to check for params in case ip is 0
	if (!self->params) {
		log_error("No params object found.");
		return -1;
	}
	json_object_object_foreach(self->params, key, val) {
		if (key[0] == '_') {
			// keys starting with _ are comments
		} else if (!strcmp(key, "ip")) {
			inet_pton(AF_INET,
				json_object_get_string(val),
				&(data->dest_sa.sin_addr)
			);
			got_params |= 1;
			log_trace("ip = 0x%X", data->dest_sa.sin_addr);
		} else if (!strcmp(key, "port")) {
			data->dest_sa.sin_port = htons(
				(unsigned short)strtoul(
					json_object_get_string(val), 0, 0
				)
			);
			got_params |= 2;
			log_trace("port = 0x%X", data->dest_sa.sin_port);
		} else {
			log_warn("Unknown parameter \"%s\"", key);
		}
	}
	// make sure we didn't miss any params
	if (got_params != (1|2)) {
		log_error("You must provide all params: ip, port.");
		return -1;
	}
	// we're not using sendto(), so we need to connect() the socket first
	int err = connect(
		data->sock,
		(struct sockaddr *)&data->dest_sa,
		sizeof(data->dest_sa)
	);
	if (err) {
		log_error("Couldn't connect: %s", strerror(errno));
		return -1;
	}
	// set types and units
	self->type_in = AYLP_T_ANY;
	self->units_in = AYLP_U_ANY;
	self->type_out = 0;
	self->units_out = 0;
	return 0;
}


int udp_sink_process(struct aylp_device *self, struct aylp_state *state)
{
	struct aylp_udp_sink_data *data = self->device_data;
	// make data contiguous
	int needs_free = get_contiguous_bytes(&data->bytes, state);
	if (needs_free < 0) return needs_free;
	// first thing we send is the aylp header
	data->iovecs[0].iov_base = &state->header;
	data->iovecs[0].iov_len = sizeof(state->header);
	// second thing we send is the block data
	data->iovecs[1].iov_base = data->bytes.data;
	data->iovecs[1].iov_len = data->bytes.size;
	// write all the data in one go
	size_t n = data->iovecs[0].iov_len + data->iovecs[1].iov_len;
	log_trace("Writing %zu bytes to UDP", n);
	ssize_t err = writev(data->sock, data->iovecs, 2);
	if (err < 0) {
		// if n > SSIZE_MAX, this will fire, so we don't need to check
		// the sign of (ssize_t)n ourselves
		log_error("Couldn't send data: %s", strerror(errno));
	} else if (err != (ssize_t)n) {
		if (err == -1) {
			log_error("Couldn't send data: %s", strerror(errno));
		} else {
			log_error("Short write: %zu of %zu", err, n);
		}
	}
	if (needs_free) {
		xfree(data->bytes.data);
	}
	return (err < 0) ? -1 : 0;
}


int udp_sink_close(struct aylp_device *self)
{
	xfree(self->device_data);
	return 0;
}

