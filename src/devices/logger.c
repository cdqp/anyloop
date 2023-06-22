#include "openao.h"
#include "logging.h"
#include "logger.h"


int logger_init(struct oao_device *self)
{
	self->process = &logger_process;
	self->close = &logger_close;
	log_info("logger initialized");
	return 0;
}


int logger_process(struct oao_device *self, struct oao_state *state)
{
	log_info("logger processed");
	return 0;
}


int logger_close(struct oao_device *self)
{
	log_info("logger closed");
	return 0;
}

