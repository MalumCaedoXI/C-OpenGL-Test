#!/bin/sh

mkdir build
glslc ./resources/shaders/baseVertexShader.vert -o ./build/baseVertexShader.spv
glslc ./resources/shaders/baseFragmentShader.frag -o ./build/baseFragmentShader.spv

cmake -S . -B ./build
cmake -DCMAKE_BUILD_TYPE=Debug --build ./build

./test.sh