The AYLP filetype
=================

Anyloop uses a very simple file format. A valid AYLP file is a bunch of
`aylp_chunk`s one after another in which each `aylp_chunk` looks like
(pseudocode):

```c
struct aylp_chunk {
	/** Header. */
	struct aylp_header header;
	/** Pipeline data. */
	union {
		// for AYLP_T_BLOCK_UCHAR or AYLP_T_MATRIX_UCHAR
		unsigned char uchars[header.log_dim.x * header.log_dim.y];
		// for AYLP_T_BLOCK, AYLP_T_VECTOR, or AYLP_T_MATRIX
		double doubles[header.log_dim.x * header.log_dim.y];
	};
};
```

where of course `aylp_header` is defined in [anyloop.h](../libaylp/anyloop.h).
Decoding an AYLP chunk thus requires parsing header of known length, using
`header.type` to find out whether the pipeline data is in uchars or doubles, and
using `header.log_dim` to determine the size of the pipeline data. You can see
an example of decoding an AYLP file in [anyloop.jl](../contrib/anyloop.jl).

