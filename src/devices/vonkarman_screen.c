#include "openao.h"
#include "logging.h"
#include "vonkarman_screen.h"


void vonkarman_screen_init(struct oao_device *self)
{
	self->process = &vonkarman_screen_process;
	self->close = &vonkarman_screen_close;
	log_trace("vonkarman_screen initialized");
	return;
}


void vonkarman_screen_process(struct oao_device *self)
{
	log_trace("vonkarman_screen processed");
	return;
}


void vonkarman_screen_close(struct oao_device *self)
{
	log_trace("vonkarman_screen closed");
	return;
}

