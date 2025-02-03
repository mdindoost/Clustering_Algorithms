#!/bin/bash

set -e  # Exit immediately if a command fails

echo "Cloning submodules..."
git submodule update --init --recursive

echo "Building igraph..."
cd external/igraph
mkdir -p build
cd build
cmake .. -DCMAKE_INSTALL_PREFIX=../../install -DCMAKE_POSITION_INDEPENDENT_CODE=ON -DBUILD_SHARED_LIBS=ON
cmake --build . --target install
cd ../../../

echo "Building libleidenalg..."
cd external/libleidenalg
mkdir -p build
cd build
cmake .. -DCMAKE_PREFIX_PATH=../../install -DCMAKE_INSTALL_PREFIX=../../install
cmake --build . --target install
cd ../../../

echo "Building main project..."
mkdir -p build
cd build
cmake ..
cmake --build .

echo "Setup complete! Run ./build/leiden_test to test Leiden clustering."

