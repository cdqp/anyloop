OpenAO
======

Flexible and extensible open source adaptive optics software.


To add a new device:

 1. write `src/devices/$MYDEVICE.c` and `src/devices/$MYDEVICE.h`
 2. modify `device.h` to include the header file
 3. add the new files to meson.build

