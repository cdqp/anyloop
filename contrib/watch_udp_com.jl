#!/usr/bin/julia
# just a plotting script to watch the output of anyloop:center_of_mass from udp
# expects matrix output on 64730 and vector output on 64731
include("anyloop.jl")
using .Anyloop
using ArgParse
using Plots; gr()
using Sockets

#argset = ArgParseSettings()
#@add_arg_table argset begin
#    "port"
#        help = "port to listen on"
#        required = true
#end
#args = parse_args(argset)
##args = Dict("port" => "64731")

sock0 = UDPSocket()
if !bind(sock0, ip"0.0.0.0", 64730)
    throw(SystemError("couldn't open port"))
end
sock1 = UDPSocket()
if !bind(sock1, ip"0.0.0.0", 64731)
    throw(SystemError("couldn't open port"))
end

println("listening on 64730 (matrix) and 64731 (vector)")

L = 10
M = 10
l = vcat([repeat([i], M) for i in 1:L]...)
m = repeat(1:M, L)

for i in 1:10000
    data0 = read(IOBuffer(recv(sock0)), AYLPChunk)
    data1 = read(IOBuffer(recv(sock1)), AYLPChunk)

    heatmap(data0.data, aspect_ratio=:equal, size=(800,800))

    @assert 2*L*M == length(data1.data)
    # output from center_of_mass device is in order [y1,x1,y2,x2,...]
    u = reshape(data1.data, (2,:))[1,:]
    v = reshape(data1.data, (2,:))[2,:]
    display(quiver!(
        8*(m.-0.5).+0.5, 8*(l.-0.5).+0.5, quiver=(4*v,4*u), color=:magenta
    ))
end

close(sock1)


