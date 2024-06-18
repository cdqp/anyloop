aylp:delay
==========

Types and units: `[T_ANY, U_ANY] -> [T_UNCHANGED, U_UNCHANGED]`.

This device pauses execution of the loop for a certain period of time.

Parameters
----------

  - `s` (integer) (optional)
    - Number of seconds to delay (default 0).
  - `ns` (integer) (optional)
    - Number of nanoseconds to delay; must be less than 1E9 (default 0).

