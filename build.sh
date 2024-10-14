#!/bin/bash
set -e

# Check for "--rebuild-deps" argument
REBUILD_DEPS=false
if [ "$1" == "--rebuild-deps" ]; then
    REBUILD_DEPS=true
fi

module load cmake
module load gcc
export igraph_DIR=$(pwd)/igraph/install/lib64/cmake/igraph

if $REBUILD_DEPS; then
    # Build igraph
    echo "Building igraph..."
    cd igraph
    rm -rf build
    mkdir build && cd build
    cmake -DCMAKE_INSTALL_PREFIX=../install -DBUILD_SHARED_LIBS=ON ..
    cmake --build . --target install
    cd ../../

    # Build libleidenalg
    echo "Building libleidenalg..."
    cd libleidenalg
    rm -rf build
    mkdir build && cd build
    cmake -DCMAKE_INSTALL_PREFIX=../install ..
    cmake --build . --target install
    cd ../../
fi

# Build your project
echo "Building Clustering_Algorithms..."
if [ ! -d "build" ]; then
    mkdir build
fi
cd build
cmake ..
make

export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$(pwd)/../igraph/install/lib64:$(pwd)/../libleidenalg/install/lib

echo "Build complete. You can run your application with:"
echo "echo "./src/clustering_app"