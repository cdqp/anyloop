#ifndef _OPENAO_DEVICE_H
#define _OPENAO_DEVICE_H

#include "devices/file_sink.h"
#include "devices/logger.h"
#include "devices/vonkarman_screen.h"

const static struct {
	const char *uri;
	int (*init_fun)(struct oao_device *);
} init_map [] = {
	{
		"openao:file_sink",
		file_sink_init,
	},
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
int init_device(struct oao_device *dev);

#endif

