anyloop:center_of_mass
======================

Types and units: `[T_MATRIX_UCHAR, U_ANY] -> [T_VECTOR, U_MINMAX]`.

This device breaks up an image into one or more regions, and calculates the
center-of-mass coordinate of that image. For example, this device might be used
with only one region to determine the center-of-mass coordinates of a beam on a
camera, which can then be used to control a tip-tilt mirror to recenter said
beam. This device is also used with many regions for getting error signals from
a wavefront sensor.

An example configuration for a wavefront sensor with 8x8-pixel subapertures:

```json
{
  "uri": "anyloop:center_of_mass",
    "params": {
      "region_height": 8,
      "region_width": 8,
      "thread_count": 1
    }
}
```

Pipeline data is replaced with a vector of interleaved center-of-mass y and x
coordinates (a vector of length 2N, where N is the number of regions of
interest). For example, if the input has four regions of interest, the output
will be [y1,x1,y2,x2,y3,x3,y4,x4] where each y,x is from -1 to 1, where 0 means
perfectly centered in the region of interest. It is assumed that the input is
written in order of increasing x coordinate, then increasing y coordinate.

Parameters
----------

- `region_height` (integer) (required)
  - Height of each region to find the center of mass of. The image will be split
    up into regions of this height, from the top going down. Excess data will be
    ignored. Set this to 0 to set the region height to the logical height of the
    whole image.
- `region_width` (integer) (required)
  - Width of each region to find the center of mass of. The image will be split
    up into regions of this width, from left to right. Excess data will be
    ignored. Set this to 0 to set the region width to the logical height of the
    whole image.
- `thread_count` (integer) (optional)
  - Number of threads to use for the calculation. Set this to 1 (default) for no
    multithreading.

