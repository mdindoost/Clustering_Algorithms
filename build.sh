#!/bin/bash
set -e

# Check for "--rebuild-deps" argument
REBUILD_DEPS=false
if [ "$1" == "--rebuild-deps" ]; then
  REBUILD_DEPS=true
fi

if [ command -v "module" ] 2>&1 >/dev/null; then
  module load GCC
  module load CMake
fi

if [ $(uname) = "Darwin" ]; then
  rm -f $(pwd)/igraph/INSTALL
fi

export igraph_DIR=$(pwd)/igraph/install/lib/cmake/igraph

if $REBUILD_DEPS; then
  # Build igraph
  echo "Building igraph..."
  cd igraph
  rm -rf build
  mkdir build && cd build
  cmake -DCMAKE_INSTALL_PREFIX=../install -DCMAKE_INSTALL_LIBDIR=lib -DBUILD_SHARED_LIBS=ON ..
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

if [ $(uname) = "Darwin" ]; then
  export DYLD_LIBRARY_PATH=$DYLD_LIBRARY_PATH:$(pwd)/../igraph/install/lib:$(pwd)/../libleidenalg/install/lib
else
  export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$(pwd)/../igraph/install/lib:$(pwd)/../libleidenalg/install/lib
fi

# Build your project
echo "Building Clustering_Algorithms..."

if [ -d "build" ]; then
  rm -rf build
fi

mkdir build
cd build

cmake ..
make

echo "Build complete. You can run your application with:"
echo "./build/src/clustering_app"
