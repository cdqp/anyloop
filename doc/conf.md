Configuration
=============

Anyloop is just a fancy `while` loop. The configuration you supply to it, in the
form of an array of devices, just tells anyloop what to run inside the loop. The
syntax of such a config file is like so:

```json
{
    "pipeline": [
        <!-- devices go here -->
    ]
}
```

At the top level of the JSON file, there is a `pipeline` array, in which devices
are added. Beyond doing some initialization at startup or destruction in the
end, devices will run a "proc" function in the order they are added to the
pipeline. What that processing looks like depends on the device; some might
acquire data to the pipeline, some might modify data in the pipeline, and some
might write data from the pipeline to an instrument or the internet, etc.

Either way, the main way for devices to hand data off to each other is through
the pipeline state (see `aylp_state` in [anyloop.h](../libaylp/anyloop.h)). The
state at any time could be a matrix, a vector, or some other type of data
structure—all that matters is that each device is okay with the type of data the
previous device outputted. See `aylp_type` and `aylp_device` in
[anyloop.h](../libaylp/anyloop.h)).

For control system use cases like adaptive optics, the pipeline is likely to
look quite complex, but should probably comprise at least the following steps:

1. a device to read an error signal into the pipeline state;
2. a device to turn the error signal in the pipeline state into a correction
   signal;
3. a device to apply the correction signal stored in the pipeline state.

Devices 1, 2, and 3 above would then proc in order indefinitely until anyloop
was interrupted.

See [devices.md](devices.md) for further documentation on how devices work.

Walkthrough
-----------

Let's start by passing in a very simple conf file, with just one device:

```json
{
    "pipeline": [
        {
            "uri": "anyloop:delay",
            "params": {
                "s": 0,
                "ns": 100000000
            }
        }
    ]
}
```

Save this file as `test.json` and run it with `anyloop -pl trace test.json`. You
should see a trace of the `anyloop:delay` device being procced ten times a
second. Let's add an `anyloop:logger` to print the pipeline state.

```json
{
    "pipeline": [
        {
            "uri": "anyloop:logger"
        },
        {
            "uri": "anyloop:delay",
            "params": {
                "s": 0,
                "ns": 100000000
            }
        }
    ]
}
```

You should now see a bunch of this:

```
WARN  ../devices/logger.c:45: Seeing type 0 but don't know how to print it
INFO  ../devices/logger.c:47: Perhaps there's no data in the pipeline?
```

Of course, if we have no devices writing to the pipeline state, the logger has
nothing to log! So let's put in an `anyloop:test_source` to feed in some sine
data.

```json
{
    "pipeline": [
        {
            "uri": "anyloop:test_source",
                "params": {
                    "type": "vector",
                    "kind": "sine",
                    "size1": "2"
                }
        },
        {
            "uri": "anyloop:logger"
        },
        {
            "uri": "anyloop:delay",
            "params": {
                "s": 0,
                "ns": 100000000
            }
        }
    ]
}
```

Now, you should see a bunch of:

```
TRACE ../libaylp/anyloop.c:249: Processed anyloop:test_source
INFO  ../devices/logger.c:26: Seeing vector of size 2:
INFO  ../devices/logger.c:27: [0.717356, 0.717356]
TRACE ../libaylp/anyloop.c:249: Processed anyloop:logger
TRACE ../libaylp/anyloop.c:249: Processed anyloop:delay
```

with the numbers from the logger varying sinusoidally with time. The
`anyloop:test_source` is outputting an `AYLP_T_VECTOR` of two elements into the
pipeline, with units `AYLP_U_MINMAX`—each element of the vector is a double
between −1.0 and +1.0.

Now, how do we save this output to a file? Try:

```json
{
    "pipeline": [
        {
            "uri": "anyloop:test_source",
                "params": {
                    "type": "vector",
                    "kind": "sine",
                    "size1": "2",
                    "frequency": "0.2"
                }
        },
        {
            "uri": "anyloop:logger"
        },
        {
            "uri": "anyloop:file_sink",
            "params": {
                "filename": "data.aylp"
            }
        },
        {
            "uri": "anyloop:delay",
            "params": {
                "s": 0,
                "ns": 100000000
            }
        }
    ]
}
```

Now, if you watch that `data.aylp` file on disk, for example with `tail -f
data.aylp | xxd`, you might see something like

```
00000000: 4159 4c50 0000 0404 0200 0000 0000 0000  AYLP............
00000010: 0100 0000 0000 0000 0000 0000 0000 0000  ................
00000020: 0000 0000 0000 0000 0000 0000 0000 0000  ................
00000030: 0000 0000 0000 0000 4159 4c50 0000 0404  ........AYLP....
00000040: 0200 0000 0000 0000 0100 0000 0000 0000  ................
00000050: 0000 0000 0000 0000 0000 0000 0000 0000  ................
00000060: 2ccb 8bcb ae8e b93f 2ccb 8bcb ae8e b93f  ,......?,......?
```

The log above represents two chunks of an AYLP file (at time of writing, with
the `AYLP_SCHEMA_VERSION=0`, meaning unstable). See [filetype.md](filetype.md)
for the structure of this binary file.

Let's stop there; we've just created a config file similar to
[conf_example2.json](../contrib/conf_example2.json). As an overview, this config
file is conceptually somewhat equivalent to the following Julia code:

```jl
f = open("data.bin", "a")
i = 0
while true
    # anyloop:test_source
    i += 1
    state = sin.(0.2*[i, i])
    # anyloop:logger
    write(f, state)
    # anyloop:file_sink
    println(state)
    # anyloop:delay
    sleep(0.1)
end
close(f)
```

Of course, the main difference is in the Julia code above, the data is written
as raw `Float64`s, whereas anyloop will write it in the AYLP format.
[filetype.md](filetype.md) has more information on the structure of the AYLP
format.

