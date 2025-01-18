#!/usr/bin/julia
# just a plotting script to watch AYLP files from udp
include("anyloop.jl")
using .Anyloop
using ArgParse
using Plots; gr()
using Sockets

argset = ArgParseSettings()
@add_arg_table argset begin
    "port"
        help = "port to listen on"
        required = true
end
args = parse_args(argset)
#args = Dict("port" => "64731")

sock = UDPSocket()
if !bind(sock, ip"0.0.0.0", parse(Int, args["port"]))
    throw(SystemError("couldn't open port"))
end

println("listening on $(args["port"]) ...")

for i in 1:100
    recvbytes = IOBuffer(recv(sock))
    chunk = read(recvbytes, AYLPChunk)
    #println(data.data)
    #display(plot(data.data))
    display(heatmap(chunk.data))
end

close(sock)

