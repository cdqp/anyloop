#!/usr/bin/julia
# just a plotting script to watch AYLP files from udp
using ArgParse
using Plots; gr()
using Sockets

struct AYLP_Header
    magic::Vector{UInt8}
    aylp_status::UInt16
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
        Matrix{UInt8},      # for gsl_block_uchar
    }
end

# TODO: optimize read() to be faster

function Base.read(io::IO, ::Type{AYLP_Header})
    magic = Vector{UInt8}(undef, 4)
    println("magic is $(sizeof(magic))")
    for i in 1:4
        magic[i] = read(io, UInt8)
    end
    @assert String(magic) == "AYLP";
    aylp_status = read(io, UInt16)
    aylp_units = read(io, UInt8)
    aylp_type = read(io, UInt8)
    log_dim_y = read(io, UInt64)
    log_dim_x = read(io, UInt64)
    pitch_y = read(io, Float64)
    pitch_x = read(io, Float64)
    return AYLP_Header(
        magic, aylp_status, aylp_units, aylp_type,
        log_dim_y, log_dim_x, pitch_y, pitch_x
    )
end

function Base.read(io::IO, ::Type{AYLP_Data})
    head = read(io, AYLP_Header)
    size = head.log_dim_y * head.log_dim_x
    println(sizeof(head))
    if head.aylp_type in [1<<1, 1<<2, 1<<3]
        # block/vector/matrix
        data = Vector{Float64}(undef, size)
        for i in 1:size
            data[i] = read(io, Float64)
        end
        return AYLP_Data(head, reshape(data, (head.log_dim_y, head.log_dim_x)))
    elseif head.aylp_type == 1<<4
        # block_uchar
        data = Vector{UInt8}(undef, size)
        for i in 1:size
            data[i] = read(io, UInt8)
        end
        return AYLP_Data(head, reshape(data, (head.log_dim_y, head.log_dim_x)))
    else
        throw(ArgumentError("unknown type $(head.aylp_type)"))
    end
end

argset = ArgParseSettings()
@add_arg_table argset begin
    "port"
        help = "port to listen on"
        required = true
end
args = parse_args(argset)

sock = UDPSocket()
if !bind(sock, ip"0.0.0.0", parse(Int, args["port"]))
#if !bind(sock, ip"0.0.0.0", 64730)
    throw(SystemError("couldn't open port"))
end

println("listening on $(args["port"]) ...")

for i in 1:100
    recvbytes = IOBuffer(recv(sock))
    data = read(recvbytes, AYLP_Data)
    display(heatmap(data.data))
end

close(sock)

