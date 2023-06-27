#include "openao.h"
#include "logging.h"


// the name of this doesn't matter, as long as it's attached to self->process
int plugin_example_process(struct oao_device *self, struct oao_state *state)
{
	log_info("Plugin example processed.");
	return 0;
}


// the name of this doesn't matter, as long as it's attached to self->close
int plugin_example_close(struct oao_device *self)
{
	return 0;
}


// this is the function that will be called by name from the main loop
int plugin_example_init(struct oao_device *self)
{
	self->process = &plugin_example_process;
	self->close = &plugin_example_close;
	return 0;
}

