#!/usr/bin/bash


mkdir -p build
cd build
cmake .. -GNinja
cmake --build . --config Release
cd ..
./build/quad.exe