#include <errno.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <json-c/json.h>
#include <netinet/in.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/uio.h>
#include "anyloop.h"
#include "logging.h"
#include "udp_sink.h"


int udp_sink_init(struct aylp_device *self)
{
	self->process = &udp_sink_process;
	self->close = &udp_sink_close;
	self->device_data = (struct aylp_udp_sink_data *)calloc(
		1, sizeof(struct aylp_udp_sink_data)
	);
	struct aylp_udp_sink_data *data = self->device_data;
	data->sock = socket(AF_INET, SOCK_DGRAM, 0);
	if (data->sock == -1) {
		log_error("Couldn't initialize socket: error %d", errno);
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
	}
	// set types
	self->type_in = AYLP_T_BLOCK | AYLP_U_ANY;
	self->type_out = 0;
	return 0;
}


int udp_sink_process(struct aylp_device *self, struct aylp_state *state)
{
	struct aylp_udp_sink_data *data = self->device_data;
	// first thing we send is the aylp header
	data->iovecs[0].iov_base = &state->header;
	data->iovecs[0].iov_len = sizeof(state->header);
	// second thing we send is the block data
	data->iovecs[1].iov_base = state->block->data;
	data->iovecs[1].iov_len = sizeof(double) * state->block->size;
	// check for size consistency
	if (state->header.log_dim.y * state->header.log_dim.x
	!= state->block->size) {
		log_error("Logical dimensions in header are %lu,%lu, but "
			"block size is %lu; refusing to sink data",
			state->header.log_dim.y, state->header.log_dim.x,
			state->block->size
		);
		return 0;
	}
	// write all the data in one go
	size_t n = data->iovecs[0].iov_len + data->iovecs[1].iov_len;
	ssize_t err = writev(data->sock, data->iovecs, 2);
	if (err < 0) {
		// if n > SSIZE_MAX, this will fire, so we don't need to check
		// the sign of (ssize_t)n ourselves
		log_error("Couldn't send data: %s", strerror(errno));
	} else if (err != (ssize_t)n) {
		if (err == -1) {
			log_error("Couldn't send data: %s", strerror(errno));
		} else {
			log_error("Short write: %d of %d", err, n);
		}
	}
	return 0;
}


int udp_sink_close(struct aylp_device *self)
{
	free(self->params); self->params = 0;
	return 0;
}

