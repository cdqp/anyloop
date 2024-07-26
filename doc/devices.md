Devices
=======

Anyloop uses a plugin-like syntax to add devices to a loop.


Anatomy of a device
-------------------

See the `aylp_device` struct in `anyloop.h`. Devices have init and fini
functions that each run once, and a proc function that runs once every loop.
Some ground rules:

 1. Devices must free any memory they allocate.
 2. Devices must not free any memory they didn't allocate.
 3. Devices must maintain copies of any memory they allocate. `state->matrix`
    (for example) should *never* be the only pointer to its data; otherwise,
    that's just begging for a memory leak.
 4. If devices fail to do their job, they must return a nonzero error code from
    `proc()`. This is especially important if the device was supposed to
    change the pipeline type.
 5. Devices should self-document their parameters and accepted/outputted
   `aylp_type`s and `aylp_units`, either in their header files or in separate
   documentation elsewhere.


Built-in devices
----------------

Devices are named with a URI-like syntax. Built-in devices start with the scheme
`anyloop`, and are followed only by a "path" (to use URI terminology). For
example, the built-in device to generate von Kármán streams has the path
`vonkarman_stream`, and thus has the URI `anyloop:vonkarman_stream`.

To add a new built-in device:

 1. write `src/devices/$MYDEVICE.c` and `src/devices/$MYDEVICE.h`
 2. modify `device.h` to include the header file
 3. add the device's initialization function to the init map in `device.h`
 4. add the new files to meson.build

Documentation for each built-in device is located under the
[doc/devices](devices) directory of this repository.


Plug-in devices
---------------

Sometimes, due to licensing requirements or whatever, it may be convenient to
use devices not in this repository. No problem! Use the `file` scheme
with no authority ([RFC 3986]). For example,
to load a plug-in device at `/opt/foo/bar.so`, write the URI as
`file:/opt/foo/bar.so`.

The plug-in device will be initialized the same way built-in devices are. As
such, a device named `bar.so` is expected to have a function of the following
name and type signature:

```c
int bar_init(struct aylp_device *self)
```

Note that we strip off anything including and after the first `.` character in
the basename, so a device with a basename of `blah.a.b.c` should have an init
function named `blah_init`.

This function can then attach whatever proc() and fini() functions it wishes
to its own `aylp_device` struct.



[RFC 3986]: https://datatracker.ietf.org/doc/html/rfc3986

