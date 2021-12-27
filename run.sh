#!/usr/bin/bash


mkdir -p build
cd build
cmake .. -GNinja
cmake --build . --config Debug
cd ..
./build/quad.exe