#include <json-c/json.h>
#include "openao.h"
#include "logging.h"
#include "udp_sink.h"


int udp_sink_init(struct oao_device *self)
{
	self->process = &udp_sink_process;
	self->close = &udp_sink_close;
	log_info("udp_sink initialized");
	return 0;
}


int udp_sink_process(struct oao_device *self, struct oao_state *state)
{
	log_trace("udp_sink processed");
	return 0;
}


int udp_sink_close(struct oao_device *self)
{
	log_info("udp_sink closed");
	return 0;
}

