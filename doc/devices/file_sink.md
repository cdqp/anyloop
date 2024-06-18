anyloop:file_sink
=================

Types and units: `[T_ANY, U_ANY] -> [T_UNCHANGED, U_UNCHANGED]`.

This device writes the current pipeline state to an AYLP file. See
[filetype.md](../filetype.md) for documentation on the AYLP file format.

Parameters
----------

- `filename` (string) (required)
  - The filename to write the pipeline data to.
- `flush` (boolean) (optional)
  - Whether or not to flush the output every iteration. Setting this may
    slightly hinder performance, but will ensure that anything reading the file
    is not waiting for a buffered write.

