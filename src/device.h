#ifndef _OPENAO_DEVICE_H
#define _OPENAO_DEVICE_H

#include "devices/delay.h"
#include "devices/file_sink.h"
#include "devices/logger.h"
#include "devices/udp_sink.h"
#include "devices/vonkarman_stream.h"

const static struct {
	const char *uri;
	int (*init_fun)(struct oao_device *);
} init_map [] = {
	{
		"openao:delay",
		delay_init
	},
	{
		"openao:file_sink",
		file_sink_init
	},
	{
		"openao:logger",
		logger_init
	},
	{
		"openao:udp_sink",
		udp_sink_init
	},
	{
		"openao:vonkarman_stream",
		vonkarman_stream_init
	},
};

// match an oao_device with its initializer function and initialize it
int init_device(struct oao_device *dev);

#endif

