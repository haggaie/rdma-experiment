#!/bin/sh
# build script to compile the RDMA example.
# After a successful completion the results should be in the src/build/ directory.

dir=$(dirname $0)

# Create a build directory
mkdir -p $dir/build
# Change the current directory to the build directory
cd $dir/build
# Use CMake to generate the build scripts (Makefile)
cmake ..
# Use GNU make to build the examples
make
