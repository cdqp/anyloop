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

