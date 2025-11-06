#!/usr/bin/env bash
set -euo pipefail
ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
PREFIX="$ROOT/external/install"
LIBDIR=lib64

mkdir -p "$PREFIX"

echo "[1/3] Build igraph"
pushd "$ROOT/external/igraph"
rm -rf build && mkdir -p build && cd build
cmake .. -DCMAKE_INSTALL_PREFIX="$PREFIX" -DBUILD_SHARED_LIBS=ON -DCMAKE_POSITION_INDEPENDENT_CODE=ON -DCMAKE_INSTALL_LIBDIR=$LIBDIR
cmake --build . --target install -j
popd

echo "[2/3] Build Arrow+Parquet (C++)"
pushd "$ROOT/external/arrow/cpp"
rm -rf build && mkdir -p build && cd build
cmake .. -DCMAKE_INSTALL_PREFIX="$PREFIX" -DCMAKE_INSTALL_LIBDIR=$LIBDIR -DARROW_PARQUET=ON -DARROW_COMPUTE=ON -DARROW_JSON=OFF -DARROW_CSV=OFF -DARROW_WITH_SNAPPY=ON -DARROW_WITH_ZLIB=ON -DARROW_BUILD_SHARED=ON -DARROW_BUILD_STATIC=OFF -DARROW_BUILD_TESTS=OFF -DARROW_SIMD_LEVEL=NONE
cmake --build . --target install -j
popd

echo "[3/3] Build libleidenalg (links to vendored igraph)"
pushd "$ROOT/external/libleidenalg"
rm -rf build && mkdir -p build && cd build
cmake .. -DCMAKE_PREFIX_PATH="$PREFIX" -Digraph_DIR="$PREFIX/$LIBDIR/cmake/igraph" -DCMAKE_INSTALL_PREFIX="$PREFIX" -DCMAKE_INSTALL_LIBDIR=$LIBDIR
cmake --build . --target install -j
popd

echo "Done. Vendored libs at: $PREFIX/$LIBDIR"
