#include <sys/ioctl.h>
#include <errno.h>
#include <fcntl.h>
#include <sched.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "anyloop.h"
#include "logging.h"
#include "menable5.h"

// do we want to consider 10-bit packed? kinda doubt it'd be worth it.


int menable5_init(struct aylp_device *self)
{
	int err;
	log_init(self->log_status);
	self->device_data = (struct aylp_menable5_data *)calloc(
		1, sizeof(struct aylp_menable5_data)
	);
	struct aylp_menable5_data *data = self->device_data;
	// attach methods
	self->process = &menable5_process;
	self->close = &menable5_close;

	// parse the params json into our data struct
	if (!self->params) {
		log_error("No params object found.");
		return -1;
	}
	json_object_object_foreach(self->params, key, val) {
		if (key[0] == '_') {
			// keys starting with _ are comments
		} else if (!strcmp(key, "width")) {
			data->width = json_object_get_uint64(val);
			log_trace("width = %E", data->width);
		} else if (!strcmp(key, "height")) {
			data->height = json_object_get_uint64(val);
			log_trace("height = %E", data->height);
		} else if (!strcmp(key, "bytes_per_px")) {
			data->bytes_per_px = json_object_get_uint64(val);
			log_trace("bytes_per_px = %E", data->bytes_per_px);
		} else {
			log_warn("Unknown parameter \"%s\"", key);
		}
	}
	// make sure we didn't miss any params
	if (!data->width || !data->height || !data->bytes_per_px) {
		log_error("You must provide the following nonzero params: "
			"width, height, bytes_per_px"
		);
		return -1;
	}

	// filling in some parameters:
	// we only need one subbuf with whatever max size
	data->creation.maxsize = UINT64_MAX;
	data->creation.subbufs = 1;
	// and we need to make enough room for that subbuf
	data->memory.length = data->width * data->height * data->bytes_per_px;
	data->memory.subnr = 0;
	// .start and .headnr uninitialized
	// we'll want to capture in blocking mode, indefinitely
	data->control.mode = DMA_BLOCKINGMODE;
	data->control.timeout = 1000;
	data->control.transfer_todo = GRAB_INFINITE;
	data->control.chan = 0;
	data->control.start_buf = 0;
	data->control.dma_dir = MEN_DMA_DIR_DEVICE_TO_CPU;
	// .head uninitialized

	// open the /dev file
	char fgpath[] = "/dev/menable0";	// TODO: json?
	data->fg = open(fgpath, O_RDWR);
	if (data->fg == -1) {
		log_error("Couldn't open %s: %s\n", fgpath, strerror(errno));
		return -1;
	}

	// allocate memory on the kernel side
	data->memory.headnr = (unsigned int)ioctl(
		data->fg,
		MEN_IOC(ALLOCATE_VIRT_BUFFER, data->creation),
		&data->creation
	);
	if ((int)data->memory.headnr < 0) {
		log_error("allocate_virt_buffer failed: %s\n",
			strerror(errno)
		);
		return -1;
	}
	log_debug("Got head number %d\n", data->memory.headnr);

	// allocate our own buffer
	data->memory.start = (unsigned long)malloc(data->memory.length);
	if (!data->memory.start) {
		log_error("malloc failed: %s\n", strerror(errno));
		return -1;
	}
	// copy the pointer to data->fb for ease of access
	data->fb = (char const *)data->memory.start;
	// data->memory is now fully initialized

	// tell the driver about our buffer
	err = ioctl(
		data->fg,
		MEN_IOC(ADD_VIRT_USER_BUFFER, data->memory),
		&data->memory
	);
	if (err) {
		log_error("add_virt_user_buffer failed: %s\n", strerror(errno));
		return -1;
	}

	// start the capture by sending the control parameters
	data->control.head = data->memory.headnr;
	err = ioctl(
		data->fg,
		MEN_IOC(FG_START_TRANSFER, data->control),
		&data->control
	);
	if (err) {
		log_error("fg_start_transfer failed: %s\n", strerror(errno));
		return -1;
	}

	// set types and units
	self->type_in = AYLP_T_ANY;
	self->units_in = AYLP_U_ANY;
	self->type_out = AYLP_T_VECTOR;
	self->units_out = AYLP_U_MINMAX;

	return 0;
}


int menable5_process(struct aylp_device *self, struct aylp_state *state)
{
	int err;
	struct aylp_menable5_data *data = self->device_data;
	struct men_io_bufidx bufidx = {
		.headnr = data->memory.headnr,
		.index = data->memory.subnr,
	};
	err = ioctl(data->fg, MEN_IOC(UNLOCK_BUFFER_NR,bufidx), &bufidx);
	if (UNLIKELY(err)) {
		log_error("Unlock failed: %s\n", strerror(errno));
		return err;
	}
	// only try 1k times to get a frame
	size_t i;
	for (i=0; i<1000; i++) {
		struct handshake_frame hand = {
			.head = data->memory.headnr,
			.mode = SEL_ACT_IMAGE
		};
		err = ioctl(data->fg,
			MEN_IOC(GET_HANDSHAKE_DMA_BUFFER,hand), &hand
		);
		if (UNLIKELY(err)) {
			log_error("Handshake failed: %s\n", strerror(errno));
			return err;
		}
		if (hand.frame == -12) {
			// frame not ready
			sched_yield();
		} else {
			break;
		}
	}
	if (UNLIKELY(i==1000)) {
		log_error("Hit max tries for frame read");
		return -1;
	}
	err = ioctl(data->fg, MEN_IOC(DMA_LENGTH,bufidx), &bufidx);
	if (UNLIKELY(err)) {
		log_error("Get length failed: %s\n", strerror(errno));
		return err;
	}
	// yes, I know, unions are a thing, but I'm using the driver's struct
	// verbatim, and this is fun and cursed ;)
	// (men_io_bufidx is technically a union of the struct and a size_t)
	size_t imglen = *(size_t *)(void *)&bufidx;
	log_trace("Got image of length: %lu\n", imglen);
	// TODO: multithreaded centroiding
	UNUSED(state);
	return 0;
}


int menable5_close(struct aylp_device *self)
{
	struct aylp_menable5_data *data = self->device_data;
	ioctl(data->fg, MEN_IOC(FG_STOP_CMD,0), data->control.chan);
	ioctl(data->fg, MEN_IOC(FREE_VIRT_BUFFER,0), data->memory.headnr);
	free((char *)data->fb); data->fb = 0;
	free(data); self->device_data = 0;
	return 0;
}

