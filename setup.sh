#!/bin/bash
set -e

echo "Cloning submodules..."
git submodule update --init --recursive

# ---------- igraph ----------
echo "Building igraph..."
cd external/igraph
mkdir -p build && cd build
cmake .. \
  -DCMAKE_INSTALL_PREFIX=../../install \
  -DCMAKE_POSITION_INDEPENDENT_CODE=ON \
  -DBUILD_SHARED_LIBS=ON \
  -DCMAKE_INSTALL_LIBDIR=lib64
cmake --build . --target install
cd ../../../

# ---------- Arrow + Parquet ----------
echo "Building Apache Arrow (C++) with Parquet..."
cd external/arrow/cpp
mkdir -p build && cd build
cmake .. \
  -DCMAKE_INSTALL_PREFIX=../../../install \
  -DCMAKE_INSTALL_LIBDIR=lib64 \
  -DARROW_PARQUET=ON \
  -DARROW_BUILD_SHARED=ON \
  -DARROW_BUILD_STATIC=OFF \
  -DARROW_DEPENDENCY_SOURCE=BUNDLED \
  -DARROW_WITH_SNAPPY=ON \
  -DARROW_WITH_ZLIB=ON \
  -DARROW_WITH_BROTLI=ON \
  -DARROW_WITH_ZSTD=ON \
  -DARROW_WITH_LZ4=ON \
  -DARROW_BUILD_TESTS=OFF \
  -DARROW_BUILD_BENCHMARKS=OFF \
  -GNinja
ninja install
cd ../../../../

# ---------- libleidenalg (optional; keep for compatibility/tests) ----------
echo "Building libleidenalg..."
cd external/libleidenalg
mkdir -p build && cd build
cmake .. \
  -DCMAKE_PREFIX_PATH=../../install \
  -DCMAKE_INSTALL_PREFIX=../../install \
  -DCMAKE_INSTALL_LIBDIR=lib64
cmake --build . --target install
cd ../../../

# ---------- main project ----------
echo "Building main project..."
mkdir -p build && cd build
cmake .. -DCMAKE_PREFIX_PATH="../external/install"
cmake --build .

echo "Setup complete! Binaries in ./build/"
