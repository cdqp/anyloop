aylp:udp_sink
=============

Types and units: `[T_ANY, U_ANY] -> [T_UNCHANGED, U_UNCHANGED]`.

This device writes the current pipeline state to a UDP port as an AYLP file. See
[filetype.md](../filetype.md) for documentation on the AYLP file format.

Parameters
----------

- `ip` (string) (required)
  - The IP address to send the data to.
- `port` (string) (required)
  - The port to send the data to.

