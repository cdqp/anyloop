#include <errno.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <json-c/json.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include "openao.h"
#include "logging.h"
#include "udp_sink.h"


int udp_sink_init(struct oao_device *self)
{
	self->process = &udp_sink_process;
	self->close = &udp_sink_close;
	self->device_data = (struct oao_udp_sink_data *)calloc(
		1, sizeof(struct oao_udp_sink_data)
	);
	struct oao_udp_sink_data *data = self->device_data;
	data->sock = socket(AF_INET, SOCK_DGRAM, 0);
	if (data->sock == -1) {
		log_error("Couldn't initialize socket: error %d", errno);
	}
	data->dest_sa.sin_family = AF_INET;
	memset(&(data->dest_sa), 0, sizeof(data->dest_sa));
	int got_params = 0;	// use this to check for params in case ip is 0
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
	return 0;
}


int udp_sink_process(struct oao_device *self, struct oao_state *state)
{
	struct oao_udp_sink_data *data = self->device_data;
	char *send_data = "test\n";
	size_t n = strlen(send_data);
	ssize_t err = sendto(
		data->sock,
		send_data,
		n,
		0,
		(struct sockaddr *)&(data->dest_sa),
		sizeof(struct sockaddr)
	);
	if (err != n) {
		if (err == -1) {
			log_error("Couldn't send data: error %d", errno);
		} else {
			log_error("Short write: %d of %d", err, n);
		}
	}
	return 0;
}


int udp_sink_close(struct oao_device *self)
{
	free(self->params); self->params = 0;
	return 0;
}

