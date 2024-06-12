#!/usr/bin/julia
# just a plotting script to watch AYLP files from /tmp/aylp.fifo
include("anyloop.jl")
using .Anyloop
using ArgParse
using Plots; gr()

argset = ArgParseSettings()
@add_arg_table argset begin
    "file"
        help = "file to plot"
        required = true
end
args = parse_args(argset)

f = open(args["file"], "r");
data = read(f, AYLP_Data)
display(heatmap(data.data))

sleep(60*10)

close(f)

