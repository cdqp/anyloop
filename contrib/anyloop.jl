# julia module for anyloop ABI

module Anyloop

export AYLP_MAGIC, AYLP_SCHEMA_VERSION,
    AYLPStatus,
    AYLP_DONE,
    AYLPType,
    AYLP_T_NONE,
    AYLP_T_BLOCK,
    AYLP_T_VECTOR,
    AYLP_T_MATRIX,
    AYLP_T_BLOCK_UCHAR,
    AYLP_T_MATRIX_UCHAR,
    AYLP_T_ANY,
    AYLPUnits,
    AYLP_U_NONE,
    AYLP_U_RAD,
    AYLP_U_MINMAX,
    AYLP_U_COUNTS,
    AYLP_U_ANY

# https://discourse.julialang.org/t/export-enum/5396/4
#= can't get this to work right
macro exported_enum(name, args...)
    if length(args) == 1 && args[1] isa Expr && args[1].head == :block
        # handle the begin ... end block syntax
        block = args[1]
        # filter out comments from the block
        enum_members = filter(x -> x isa Symbol, block.args)
        return esc(quote
            @enum $name begin
                $(enum_members...)
            end
            export $name
            $([:(export $arg) for arg in enum_members]...)
        end)
    else
        # handle the standard @enum syntax
        return esc(quote
            @enum($name, $(args...))
            export $name
            $([:(export $arg) for arg in args]...)
        end)
    end
end
=#

AYLP_MAGIC::UInt32 = 0x504C5941
AYLP_SCHEMA_VERSION::UInt8 = 0

@enum AYLPStatus begin
    AYLP_DONE = 1 << 0
end

@enum AYLPType begin
    AYLP_T_NONE = 1 << 0
    AYLP_T_BLOCK = 1 << 1
    AYLP_T_VECTOR = 1 << 2
    AYLP_T_MATRIX = 1 << 3
    AYLP_T_BLOCK_UCHAR = 1 << 4
    AYLP_T_MATRIX_UCHAR = 1 << 5
    AYLP_T_ANY = 0xFF
end

@enum AYLPUnits begin
    AYLP_U_NONE = 1 << 0
    AYLP_U_RAD = 1 << 1
    AYLP_U_MINMAX = 1 << 2
    AYLP_U_COUNTS = 1 << 3
    AYLP_U_ANY = 0xFF
end


export AYLPHeader, AYLPChunk, AYLPFile

mutable struct AYLPHeader
    magic::UInt32
    aylp_version::UInt8
    aylp_status::UInt8
    aylp_type::UInt8
    aylp_units::UInt8
    log_dim_y::UInt64
    log_dim_x::UInt64
    pitch_y::Float64
    pitch_x::Float64
end

mutable struct AYLPChunk
    head::AYLPHeader
    data::Union{
        Matrix{Float64},    # for gsl_block, gsl_vector, or gsl_matrix
        Matrix{UInt8},      # for gsl_block_uchar or gsl_matrix_uchar
    }
end


# TODO: optimize read() to be faster; don't read so many separate times from io

function Base.read(io::IO, ::Type{AYLPHeader})
    magic = read(io, UInt32)
    @assert magic == AYLP_MAGIC
    aylp_version = read(io, UInt8)
    aylp_status = read(io, UInt8)
    aylp_units = read(io, UInt8)
    aylp_type = read(io, UInt8)
    log_dim_y = read(io, UInt64)
    log_dim_x = read(io, UInt64)
    pitch_y = read(io, Float64)
    pitch_x = read(io, Float64)
    return AYLPHeader(
        magic, aylp_version, aylp_status, aylp_units, aylp_type,
        log_dim_y, log_dim_x, pitch_y, pitch_x
    )
end

function Base.read(io::IO, ::Type{AYLPChunk})
    head = read(io, AYLPHeader)
    size = head.log_dim_y * head.log_dim_x
    if AYLPType(head.aylp_type) in [AYLP_T_VECTOR, AYLP_T_BLOCK, AYLP_T_MATRIX]
        data = Vector{Float64}(undef, size)
        for i in 1:size
            data[i] = read(io, Float64)
        end
        return AYLPChunk(head,
            # we have to do this tricky transpose of reshape() because anyloop
            # uses row-major ordering, and julia uses column-major
            Matrix{Float64}(reshape(data, (head.log_dim_x, head.log_dim_y))')
        )
    elseif AYLPType(head.aylp_type) in [AYLP_T_BLOCK_UCHAR, AYLP_T_MATRIX_UCHAR]
        data = Vector{UInt8}(undef, size)
        for i in 1:size
            data[i] = read(io, UInt8)
        end
        return AYLPChunk(head,
            # same deal with column-major
            Matrix{UInt8}(reshape(data, (head.log_dim_x, head.log_dim_y))')
        )
    else
        throw(ArgumentError("unknown type $(head.aylp_type)"))
    end
end

function Base.write(io::IO, x::AYLPHeader)
    @assert x.magic == AYLP_MAGIC
    n = write(io, x.magic)
    n += write(io, x.aylp_version)
    n += write(io, x.aylp_status)
    n += write(io, x.aylp_type)
    n += write(io, x.aylp_units)
    n += write(io, x.log_dim_y)
    n += write(io, x.log_dim_x)
    n += write(io, x.pitch_y)
    n += write(io, x.pitch_x)
    @assert n == sizeof(AYLPHeader)
    return n
end

# TODO: a lot of this is kinda sloppy tbh, make it faster if it ever matters

function Base.write(io::IO, x::AYLPChunk)
    n = write(io, x.head)
    size = x.head.log_dim_y * x.head.log_dim_x
    if AYLPType(x.head.aylp_type) in [
        AYLP_T_VECTOR, AYLP_T_BLOCK, AYLP_T_MATRIX
    ]
        @assert x.data isa Matrix{Float64}
        for r in eachrow(x.data)
            n += write(io, r)
        end
        @assert n == sizeof(AYLPHeader) + 8 * size
        return n
    elseif AYLPType(x.head.aylp_type) in [
        AYLP_T_BLOCK_UCHAR, AYLP_T_MATRIX_UCHAR
    ]
        @assert x.data isa Matrix{UInt8}
        for r in eachrow(x.data)
            n += write(io, r)
        end
        @assert n == sizeof(AYLPHeader) + size
        return n
    else
        throw(ArgumentError("unknown type $(x.head.aylp_type)"))
    end
end

end # module

