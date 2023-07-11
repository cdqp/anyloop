#!/usr/bin/julia
# just a rudimentary plotting script to watch AYLPDATA files from udp
using ArgParse
using Plots; gr()
using Sockets

struct AYLP_Header
    magic::Vector{Char}
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
    data::Vector{Float64}
end

function Base.read(io::IO, ::Type{AYLP_Header})
    magic = Vector{Char}(undef, 4)
    for i in 1:4
        magic[i] = read(io, Char)
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
    data = Vector{Float64}(undef, size)
    for i in 1:size
        data[i] = read(io, Float64)
    end
    return AYLP_Data(head, data)
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

for i in 1:50
    recvbytes = IOBuffer(recv(sock))
    data = read(recvbytes, AYLP_Data)
    show(data)
    p = heatmap(reshape(data.data, (data.head.log_dim_y, data.head.log_dim_x)))
    display(p)
end

close(sock)

