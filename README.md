anyloop
=======

Simple, extensible, plugin-based feedback loop software, with a primary focus on
supporting adaptive optics applications.

For now, anyloop aims to support:

- Linux (though it may be compatible with other POSIX systems)
- x86-64 and AArch64
- GCC and Clang

Anyloop is subject to the terms of the Mozilla Public License, version 2.0. If a
copy of the MPL was not distributed with this repository, you can obtain one at
<http://mozilla.org/MPL/2.0/>.


Installing
----------

You will need the following packages to compile and run anyloop from source:

- meson
- pkgconf
- json-c
- gsl

For example:

```sh
# archlinux (TODO: AUR package)
sudo pacman -S --asdeps meson pkgconf json-c gsl
# alpine
doas pkg add meson pkgconf json-c json-c-dev gsl gsl-dev
# raspbian
sudo apt install meson pkgconf libjson-c5 libjson-c-devel libgsl25 libgsl-devel
```

Once you have satisfied dependencies, build anyloop with meson:

```sh
meson setup build
meson compile -C build
sudo meson install -C build
```


Uninstalling
------------

Soon, anyloop will at least be an AUR package so those using Archlinux can
simply uninstall via pacman. If you installed with meson, manually wipe the
`/opt/anyloop` directory and the binary at the path returned by `which anyloop`.


How it works
------------

Inspired by multimedia frameworks like [PipeWire] and [GStreamer], anyloop
implements a [pipeline]. Unlike those more complex frameworks, anyloop is much
simpler and more generic. *Devices* (elements of the pipeline) are arranged and
processed linearly, each of them reading and writing to the data currently in
the pipeline *state*.

See [conf.md] for documentation on how to configure anyloop's pipeline, and
[devices.md] for information on how devices work.


[PipeWire]: https://pipewire.org
[GStreamer]: https://gstreamer.freedesktop.org
[pipeline]: https://en.wikipedia.org/wiki/Pipeline_(computing)
[conf.md]: doc/conf.md
[devices.md]: doc/devices.md

