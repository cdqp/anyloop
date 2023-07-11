#include "anyloop.h"
#include "logging.h"
#include "logger.h"


int logger_init(struct aylp_device *self)
{
	self->process = &logger_process;
	self->close = &logger_close;
	log_info("logger initialized");
	// set types and units
	self->type_in = AYLP_T_ANY;
	self->units_in = AYLP_U_ANY;
	self->type_out = 0;
	self->units_out = 0;
	return 0;
}


int logger_process(struct aylp_device *self, struct aylp_state *state)
{
	UNUSED(self);
	UNUSED(state);
	log_info("logger processed");
	return 0;
}


int logger_close(struct aylp_device *self)
{
	UNUSED(self);
	log_info("logger closed");
	return 0;
}

