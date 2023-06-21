#include "openao.h"
#include "logging.h"
#include "logger.h"


void logger_init(struct oao_device *self)
{
	self->process = &logger_process;
	self->close = &logger_close;
	log_trace("logger initialized");
	return;
}


void logger_process(struct oao_device *self, struct oao_state *state)
{
	log_debug("logger_process called");
	log_trace("logger processed");
	return;
}


void logger_close(struct oao_device *self)
{
	log_trace("logger closed");
	return;
}

