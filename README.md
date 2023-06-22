OpenAO
======

Simple, extensible, open-source adaptive optics software.

To add a new device:

 1. write `src/devices/$MYDEVICE.c` and `src/devices/$MYDEVICE.h`
 2. modify `device.h` to include the header file
 3. add the device's initialization function to the init map in `device.h`
 4. add the new files to meson.build


TODO
----

- doxygen
- unit tests
- actual physical dm control
- actual physical wfs control
- 2d resampling
- ... and much more

