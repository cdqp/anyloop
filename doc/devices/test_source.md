anyloop:test_source
===================

Types and units: `[T_ANY, U_ANY] -> [
  T_VECTOR|T_MATRIX|T_MATRIX_UCHAR,
  U_MINMAX|U_MINMAX|U_COUNTS
]`.

This device generates test data into the pipeline.

Parameters
----------

- `type` (string) (required)
  - `vector` if we want to generate `T_VECTOR`s
  - `matrix` if we want to generate `T_MATRIX`s
  - `matrix_uchar` if we want to generate `T_MATRIX_UCHAR`s
- `kind` (string) (required)
  - "constant" if we want to just output a bunch of constants
  - "sine" if we want our output to time-vary sinusoidally
- `size1` (integer) (required)
  - Size of output vector or height of output matrix.
- `size2` (integer) (required if type is `matrix` or `matrix_uchar`)
  - Width of output matrix.
- `frequency` (float) (optional)
  - Frequency of sinusoidal oscillation, in units of radians per loop
    iteration. Defaults to 0.1.
- `amplitude` (float) (optional)
  - Amplitude of sinusoidal oscillation. Defaults to 1.0.
  - Note that for `vector` and `matrix` types, this device outputs units of
    `AYLP_U_MINMAX`; the output will be clipped to ±1.0. For the
    `matrix_uchar` type, the output is instead clipped between 0:255.
- `offset` (float) (optional)
  - DC offset of sinusoidal oscillation, or value to write every loop for
    kind == "constant".

