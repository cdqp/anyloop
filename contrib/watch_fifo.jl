#!/usr/bin/julia
# just a plotting script to watch AYLP files from /tmp/aylp.fifo
include("anyloop.jl")
using .Anyloop
using Plots; gr()

fifo_path = "/tmp/aylp.fifo"

if ispath(fifo_path)
    rm(fifo_path)
end
run(`mkfifo $(fifo_path)`)

if !isfifo(fifo_path)
    throw(SystemError("unable to create named pipe at $(fifo_path)"))
end

# r+ so we hold the fifo open so we don't read EOF
fifo = open(fifo_path, "r+");
println("pipe opened; waiting for data ...")

while true
    data = read(fifo, AYLP_Data)
    display(heatmap(data.data))
end

close(fifo)

