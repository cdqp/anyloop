Command vector conventions
==========================

Many devices are controlled by *command vectors.* These are simply
`gsl_vector` (`AYLP_T_VECTOR`) objects that describe commands to be sent to
devices downstream. Since anyloop is built on a pipeline system, one wonders how
to control multiple devices when a reconstructor only generates one command
vector.

Reconstruction
--------------

For example, an adaptive optics system may have:

1. a fine steering mirror (*N*₁=2 degrees of freedom),
2. a low-resolution (*N*₂ actuators) "woofer" deformable mirror, and
3. a high-resolution (*N*₃ actuators) "tweeter" deformable mirror.

We will likely want to modally partition them like the following:

1. send *N*₁ (2) commands corresponding to tip and tilt to the steering mirror,
2. send *N*₂ commands corresponding to low-order Zernike (or Fourier) modes to
the woofer,
3. send *N*₃ commands corresponding to high-order Zernike (or Fourier) modes to
the tweeter.

Let:

- ***E*** denote a measured error vector,
- *Rₘ* denote a modal reconstructor matrix (which converts an error vector into
  corrections in a modal basis),
- *Aₘ*₁ through *Aₘ*₃ denote matrices that convert from modal basis to actuator
  basis for each device,
- ***v****ₘ* = [***v****ₘ*₁', ***v****ₘ*₂', ***v****ₘ*₃']' denotes the total
  modal correction to be applied to all devices, and
- ***v****ₐ*₁ through ***v****ₐ*₃ denote the actuator-basis commands to be sent
  to each device.

What we essentially want to do in the AO loop is to execute these steps:

1. calculate the total modal correction to be applied: ***v****ₘ* = *Rₘ* ***E***
2. project out each device's actuator-space command from subvectors of the total
modal correction:
    - ***v****ₐ*₁ = *Aₘ*₁ ***v****ₘ*₁
    - ***v****ₐ*₂ = *Aₘ*₂ ***v****ₘ*₂
    - ***v****ₐ*₃ = *Aₘ*₃ ***v****ₘ*₃
3. send the commands ***v****ₐ*₁ through ***v****ₐ*₃ to each respective device.

What you may notice is that by forming one big matrix *Aₘ* with *Aₘ*₁ through
*Aₘ*₃ on its diagonals, we can reduce the whole thing into one big matrix
multiplication ***v****ₐ* = *Rₐ* ***E***, where *Rₐ* = diag(*Aₘ*₁, *Aₘ*₂, *Aₘ*₃)
*Rₘ*.

As such, we actually can do this all in *one* reconstructor device that simply
multiplies the error vector in the pipeline with *Rₐ*. Devices downstream should
then properly take parameters named something like `start_index` so we can tell
them how far into the ***v****ₐ* vector to skip before their actuator-basis
command actually starts.

Calibration
-----------

The question then becomes how to generate the matrices *Rₘ* and *Aₘ*. One
*could* simply generate a poke matrix and take the SVD-pseudoinverse of it to
get *Rₐ*, but I suspect that with a woofer-tweeter system there would be a lot
of degeneracies. Hence it is probably better to poke *modes*,[^modes] rather
than individual actuators. So fix a modal basis *Aₘ⁻¹*, form a poke matrix where
each column corresponds to the error signal read from a single mode, and
psuedoinvert that to get *Rₘ*. Then form *Rₐ* = diag(*Aₘ*₁, *Aₘ*₂, *Aₘ*₃) *Rₘ*
as explained previously, and write that to a file.

[^modes]: Note that there is a bit of an ambiguity in simply saying "modal" vs
"zonal." There are modes in actuator-space, and modes in sensor-space, and they
may not necessarily align. It is simpler to poke modes in actuator space, as the
*Aₘ* matrices and their inverses are mathematically known, but it is conceivable
that a pre-calibration step could be executed that would calculate these *Aₘ*
matrices and their inverses based on sensor-space modes.

