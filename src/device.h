#ifndef _OPENAO_DEVICE_H
#define _OPENAO_DEVICE_H

#include "devices/logger.h"
#include "devices/vonkarman_screen.h"

const static struct {
	const char *uri;
	void (*init_fun)(struct oao_device *);
} init_map [] = {
	{
		"openao:logger",
		logger_init
	},
	{
		"openao:vonkarman_screen",
		vonkarman_screen_init
	},
};

// match an oao_device with its initializer function and initialize it
void init_device(struct oao_device *dev);

#endif

