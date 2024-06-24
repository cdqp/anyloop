anyloop:clamp
=============

Types and units: `[T_BLOCK|T_VECTOR|T_MATRIX|T_BLOCK_UCHAR|T_MATRIX_UCHAR,
U_ANY] -> [T_UNCHANGED, U_UNCHANGED]`.

This device limits data in the pipeline to a minimum and maximum value.

Parameters
----------

- `type` (string) (required)
  - The type of data in the pipeline.
- `min` (double) (optional)
  - Minimum value to clamp to. Defaults to -1.0.
- `max` (double) (optional)
  - Maximum value to clamp to. Defaults to 1.0.

