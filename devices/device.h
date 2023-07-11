#ifndef _AYLP_DEVICE_H
#define _AYLP_DEVICE_H

#include "devices/delay.h"
#include "devices/file_sink.h"
#include "devices/logger.h"
#include "devices/stop_after_count.h"
#include "devices/udp_sink.h"
#include "devices/vonkarman_stream.h"

static const struct {
	const char *uri;
	int (*init_fun)(struct aylp_device *);
} init_map [] = {
	{
		"anyloop:delay",
		delay_init
	},
	{
		"anyloop:file_sink",
		file_sink_init
	},
	{
		"anyloop:logger",
		logger_init
	},
	{
		"anyloop:stop_after_count",
		stop_after_count_init
	},
	{
		"anyloop:udp_sink",
		udp_sink_init
	},
	{
		"anyloop:vonkarman_stream",
		vonkarman_stream_init
	},
};

// match an aylp_device with its initializer function and initialize it
int init_device(struct aylp_device *dev);

#endif

