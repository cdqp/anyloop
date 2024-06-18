#!/usr/bin/julia
# just a rudimentary plotting script to view square matrices
using ArgParse
using Plots; gr()

argset = ArgParseSettings()
@add_arg_table argset begin
    "path"
        help = "path to dump file"
        required = true
end
args = parse_args(argset)
#args = Dict{String,String}("path" => "../build/screen.bin")

MN = stat(args["path"]).size/8
M = Int(sqrt(MN))

mat = Matrix{Float64}(undef, M, M)
read!(args["path"], mat)
println("data imported")

heatmap(mat)
savefig("screen.bin.png")

