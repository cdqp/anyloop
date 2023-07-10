// this plugin fetches frames from https://github.com/cdqp/menable-driver
#ifndef _AYLP_MENABLE5_H
#define _AYLP_MENABLE5_H

#include <asm/ioctl.h>

#include "anyloop.h"

// macro for sending ioctls
// Note that the kernel driver doesn't actually check if the ioctl is _IOC_READ
// or _IOC_WRITE, so we'll use _IOC_READ which makes more sense to me. This is
// despite the menable SDK using _IOC_WRITE.
#define MEN_IOC(nr,sz)			_IOR('m', nr, sz)

// ioctl codes
// https://github.com/cdqp/menable-driver/blob/main/linux/men_ioctl_codes.h
#define FREE_VIRT_BUFFER		0x1B
#define FG_STOP_CMD			0x1B
#define DMA_LENGTH			0x21
#define UNLOCK_BUFFER_NR		0x28
#define GET_HANDSHAKE_DMA_BUFFER	0x2A
#define DMA_FRAME_NUMBER		0x2E
#define FG_START_TRANSFER		0x95
#define ALLOCATE_VIRT_BUFFER		0x98
#define ADD_VIRT_USER_BUFFER		0xA9

/* // some more codes from the driver:
 * switch(acqmode) {
 * case DMA_CONTMODE		: return "standard";
 * case DMA_BLOCKINGMODE	: return "blocking";
 * case DMA_PULSEMODE		: return "pulse";
 * case DMA_SELECTIVEMODE	: return "selective";
 * default			: return "UNKNWON";
 * }
 */
#define DMA_CONTMODE		0x10
#define DMA_HANDSHAKEMODE	0x20
#define DMA_BLOCKINGMODE	DMA_HANDSHAKEMODE
#define DMA_PULSEMODE		0x30
#define DMA_SELECTIVEMODE	0x04
/*
 * enum men_dma_dir {
 * 	MEN_DMA_DIR_DEVICE_TO_CPU,
 * 	MEN_DMA_DIR_CPU_TO_DEVICE
 * };
 */
#define MEN_DMA_DIR_DEVICE_TO_CPU 0
#define MEN_DMA_DIR_CPU_TO_DEVICE 1
// tells the driver to grab indefinitely
#define GRAB_INFINITE		-1
// whether to choose the current image or the oldest one
#define SEL_ACT_IMAGE	0xC8	// get current image
#define SEL_NEXT_IMAGE	0xDC	// get oldest image

// the following structs are also from the kernel driver
struct mm_create_s {
	uint64_t maxsize;	// this isn't actually checked by the driver lol
	long subbufs;		// how many sub-buffers to allow
};

struct men_io_range {
	unsigned long start;	// starting address in virtual memory
	unsigned long length;	// length of buffer in bytes
	long subnr;		// id of the sub-buffer
	unsigned int headnr;	// returned from ALLOCATE_VIRT_BUFFER
} __attribute__ ((packed));

struct fg_ctrl {
	int mode;		// mode of acquisition (see DMA_*MODE above)
	int timeout;		// delay for timer restarts [s]
	long transfer_todo;	// how many bytes to transfer
	unsigned int chan;	// index of dma channel (0 for us)
	unsigned int head;	// same as men_io_range.headnr?
	long start_buf;		// id of the starting sub-buffer
	int dma_dir;		// direction of dma (see MEN_DMA_DIR* above)
};

struct handshake_frame {
	union {
		struct {
			unsigned int head;	// same as men_io_range.headnr
			unsigned int mode;	// see SEL_*_IMAGE above
		};
		// this is the return value of the driver; it is -12 if there is
		// no frame available, or otherwise is the index of the frame
		// (sub-buffer index)
		long long frame;
	};
};

struct men_io_bufidx {
	unsigned int headnr;	// same as men_io_range.headnr
	long index;		// index of the buffer
};

struct aylp_menable5_data {
	// params from json
	size_t width;
	size_t height;
	size_t bytes_per_px;
	// parameters for initialization
	struct mm_create_s creation;
	struct men_io_range memory;
	struct fg_ctrl control;
	// file descriptor to the frame grabber
	int fg;
	// framebuffer (gets put into the state every round)
	char const *fb;
};

// initialize menable5 device
// (note that the fpga must already have the correct applet selected)
int menable5_init(struct aylp_device *self);

// busy-wait for frame
int menable5_process(struct aylp_device *self, struct aylp_state *state);

// close menable5 device when loop exits
int menable5_close(struct aylp_device *self);

#endif

