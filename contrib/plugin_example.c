#include "anyloop.h"
#include "logging.h"


// the name of this doesn't matter, as long as it's attached to self->proc
int plugin_example_proc(struct aylp_device *self, struct aylp_state *state)
{
	UNUSED(self);
	UNUSED(state);
	log_info("Plugin example procced.");
	return 0;
}


// the name of this doesn't matter, as long as it's attached to self->fini
int plugin_example_fini(struct aylp_device *self)
{
	UNUSED(self);
	return 0;
}


// this is the function that will be called by name from the main loop
int plugin_example_init(struct aylp_device *self)
{
	self->proc = &plugin_example_proc;
	self->fini = &plugin_example_fini;
	// set types and units
	self->type_in = AYLP_T_ANY;
	self->units_in = AYLP_U_ANY;
	self->type_out = 0;
	self->units_out = 0;
	return 0;
}

