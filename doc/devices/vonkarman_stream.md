aylp:vonkarman_stream
=====================

Types and units: `[T_ANY, U_ANY] -> [T_MATRIX, U_RAD]`.

This device fills the pipeline with a matrix of simulated von Kármán
turbulence. A large static phase screen is generated at startup, and (in
keeping with the frozen flow approximation) a small window is slid along this
phase screen as the loop runs. The contents of this window are then put into
the pipeline. This device has been used in conjunction with a deformable
mirror to simulate atmospheric turbulence in the lab.

See: <https://doi.org/10.1364/AO.43.004527>,
<https://doi.org/10.1117/12.279029>.

Parameters
----------

- `L0` (float) (required)
  - The outer scale length of the turbulence that you want to simulate.
- `r0` (float) (required)
  - The Fried parameter of the turbulence that you want to simulate.
- `pitch` (float) (required)
  - The physical distance between pixels on the phase screen (in meters).
- `screen_size` (integer) (required)
  - Width (and height) of the square phase screen. Submatrices of this large
    screen will be placed into the pipeline.
- `win_height` (integer) (optional)
  - Height of the window into the phase screen. Defaults to 10.
- `win_width` (integer) (optional)
  - Width of the window into the phase screen. Defaults to 10.
- `start_y` (integer) (optional)
  - Starting y-index of the window in the phase screen. Defaults to 0.
- `start_x` (integer) (optional)
  - Starting x-index of the window in the phase screen. Defaults to 0.
- `step_y` (integer) (optional)
  - How much to move the window in the y-direction every iteration. Proportional
    to the vertical component of wind speed under the frozen flow approximation.
    Defaults to 0.
- `step_x` (integer) (optional)
  - How much to move the window in the x-direction every iteration. Proportional
    to the horizontal component of wind speed under the frozen flow
    approximation. Defaults to 0.

