anyloop:poke
============

Types and units: `[T_VECTOR, U_MINMAX] -> [T_VECTOR, U_MINMAX]`.

This device generates a new *command vector* every iteration, and then reads in
an associated *error vector* on the next iteration; these error vectors are then
horizontally concatenated to form a *poke matrix*, which is then written to a
file. As such, this device wants to be in the pipeline before a device that will
actuate on the outputted command vectors, and after a device that will produce
some error vector.

Specifically, what this device will do, is generate command vectors that look
like ...

- iteration 1: [ 1,  0,  0,  0, ...]
- iteration 2: [-1,  0,  0,  0, ...]
- iteration 3: [ 0,  1,  0,  0, ...]
- iteration 4: [ 0, -1,  0,  0, ...]
- iteration 3: [ 0,  0,  1,  0, ...]
- iteration 4: [ 0,  0, -1,  0, ...]
- etc.

... and for every pair of iterations, it will read the two associated error
vectors, subtract one from the other, divide by two, and copy the result into
the corresponding column of the poke matrix. For example, let's say we have just
a tip-tilt mirror that takes a two-element command vector, and some other set of
devices that output the y,x coordinates of a spot on some sensor. aylp:poke
might then do the following:

1. output [1, 0] as command vector;
2. read (e.g.) [0.26, 0.26] as error vector;
3. output [-1, 0] as command vector;
4. read [-0.24, -0.24] as error vector;
5. store [0.25, 0.25] as the first column of the poke matrix;
6. output [0, 1] as command vector;
7. read [0.26, -0.26] as error vector;
8. output [0, -1] as command vector;
9. read [-0.24, 0.24] as error vector;
10. store [0.25, -0.25] as the first column of the poke matrix.

The final poke matrix in this hypothetical case would then be

```
[ 0.25  0.25
  0.25 -0.25 ]
```

Thus, we would be able to tell that the tip/tilt axes of the mirror were not
perfectly aligned to those of whatever instrument was reporting the tip/tilt
error, and this poke matrix could be used to help correct that.

Here's an example of a stripped-down configuration to generate a poke matrix for
a wavefront sensor and a deformable mirror:

```json
{ "pipeline": [
  {
    "_comment": "this is a wavefront sensor that outputs an image matrix",
    "uri": "file:aylp_basler_fgsdk.so",
    "params": {
      "width": 80,
      "height": 80
    }
  },
  {
    "uri": "anyloop:center_of_mass",
    "params": {
      "region_height": 8,
      "region_width": 8,
      "thread_count": 1
    }
  },
  {
    "uri": "anyloop:poke",
    "params": {
      "n_act": 97,
      "filename": "poke.aylp"
    }
  },
  {
    "_comment": "this is a deformable mirror device",
    "uri": "file:aylp_asdk_dm.so",
    "params": {
      "_comment": "whatever parameters needed for the deformable mirror"
    }
  }
]}
```

See [command.md](../command.md) for more information.

Parameters
----------

- `n_act` (integer) (required)
  - The count of actuators in our poking device, or in other words, the length
    of our poke matrix.
- `filename` (string) (required)
  - The filename to use when saving the poke matrix as an AYLP file.


