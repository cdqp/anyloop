#!/usr/bin/julia
# pseudoinvert a poke matrix
include("anyloop.jl")
using .Anyloop
using ArgParse
using LinearAlgebra
using Plots; gr()

argset = ArgParseSettings()
@add_arg_table argset begin
    "infile"
        help = "the input poke matrix .aylp file"
        required = true
    "outfile"
        help = "the output control matrix .aylp file"
        required = true
    "threshold"
        help = "threshold for pinv (default 0.1)"
        required = false
        default = 0.1
end
args = parse_args(argset)

fi = open(args["infile"], "r");
chunk = read(fi, AYLPChunk)
tmp = pinv(chunk.data, 0.1)
chunk.data = tmp
@assert size(chunk.data) == (chunk.head.log_dim_x, chunk.head.log_dim_y)
chunk.head.log_dim_y, chunk.head.log_dim_x = size(chunk.data)

fo = open(args["outfile"], "w");
write(fo, chunk)

close(fi)
close(fo)

