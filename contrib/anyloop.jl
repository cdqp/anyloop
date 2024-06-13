# julia module for anyloop ABI

module Anyloop

export AYLP_Header, AYLP_Data

struct AYLP_Header
    magic::Vector{UInt8}
    aylp_version::UInt8
    aylp_status::UInt8
    aylp_type::UInt8
    aylp_units::UInt8
    log_dim_y::UInt64
    log_dim_x::UInt64
    pitch_y::Float64
    pitch_x::Float64
end

struct AYLP_Data
    head::AYLP_Header
    data::Union{
        Matrix{Float64},    # for gsl_block, gsl_vector, or gsl_matrix
        Matrix{UInt8},      # for gsl_block_uchar or gsl_matrix_uchar
    }
end

# TODO: optimize read() to be faster

function Base.read(io::IO, ::Type{AYLP_Header})
    magic = Vector{UInt8}(undef, 4)
    for i in 1:4
        magic[i] = read(io, UInt8)
    end
    @assert String(magic) == "AYLP";
    aylp_version = read(io, UInt8)
    aylp_status = read(io, UInt8)
    aylp_units = read(io, UInt8)
    aylp_type = read(io, UInt8)
    log_dim_y = read(io, UInt64)
    log_dim_x = read(io, UInt64)
    pitch_y = read(io, Float64)
    pitch_x = read(io, Float64)
    return AYLP_Header(
        magic, aylp_version, aylp_status, aylp_units, aylp_type,
        log_dim_y, log_dim_x, pitch_y, pitch_x
    )
end

function Base.read(io::IO, ::Type{AYLP_Data})
    head = read(io, AYLP_Header)
    size = head.log_dim_y * head.log_dim_x
    if head.aylp_type in [1<<1, 1<<2, 1<<3]
        # block/vector/matrix
        data = Vector{Float64}(undef, size)
        for i in 1:size
            data[i] = read(io, Float64)
        end
        return AYLP_Data(head,
            # we have to do this tricky transpose of reshape() because anyloop
            # uses row-major ordering, and julia uses column-major
            Matrix{Float64}(reshape(data, (head.log_dim_x, head.log_dim_y))')
        )
    elseif head.aylp_type in [1<<4, 1<<5]
        # block_uchar/matrix_uchar
        data = Vector{UInt8}(undef, size)
        for i in 1:size
            data[i] = read(io, UInt8)
        end
        return AYLP_Data(head,
            # same deal with column-major
            Matrix{UInt8}(reshape(data, (head.log_dim_x, head.log_dim_y))')
        )
    else
        throw(ArgumentError("unknown type $(head.aylp_type)"))
    end
end

end

