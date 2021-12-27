#!/usr/bin/bash


mkdir -p build
cd build
cmake -DCMAKE_BUILD_TYPE=Release .. -GNinja
cmake --build . --config Release
cd ..
./build/quad.exe