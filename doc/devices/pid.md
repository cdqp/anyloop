anyloop:pid
===========

Types and units: `[T_VECTOR|T_MATRIX, U_ANY] -> [T_UNCHANGED, U_UNCHANGED]`.

This device applies [PID](https://en.wikipedia.org/wiki/PID_controller) error
correction, taking the pipeline state as error input and replacing it with the
calculated correction.

Let `x_0` be the current input, `x_1` be the input from last iteration of the
loop, `dt` be the time in seconds that has elapsed since the previous iteration,
`x_acc` be the accumulated total errors since the loop was started, and `y` be
the output. Then this PID device more or less implements:

```
y = - p*x_0 - i*x_acc - d*(x_0-x_1)/dt
```

More precisely, it implements the above equation but clamping the total
correction to the `clamp` parameter in magnitude, after first clamping the
`x_acc` term the same way.

Parameters
----------

- `type` (string) (required)
  - "vector" if we are expecting `T_VECTOR` input, and "matrix" if we are
    expecting `T_MATRIX` input.
- `p` (float) (optional)
  - Coefficent for proportional correction (see equation above). Defaults
    to 1.0.
- `i` (float) (optional)
  - Coefficent for integral correction (see equation above). Default 0.0.
- `d` (float) (optional)
  - Coefficent for derivative correction (see equation above). Default 0.0.
- `clamp` (float) (optional)
  - What to clamp the correction to in magnitude. Will be applied first to
    `x_acc` and later to the whole correction. Useful to prevent the integral
    component from completely running away. Defaults to 1.0.

