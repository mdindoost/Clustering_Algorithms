# Clustering Algorithms with Leiden Community Detection

This repository provides an implementation of clustering algorithms, including an interface to the Leiden algorithm using `libleidenalg` and `igraph`.

## **Installation Guide**

### **1. Clone the Repository**
To get started, clone this repository along with its submodules:

```bash
git clone --recursive https://github.com/mdindoost/Clustering_Algorithms.git
cd Clustering_Algorithms

If you already cloned the repository without submodules, initialize and update them manually:

git submodule update --init --recursive

### **2. Build and Install Dependencies**

The project relies on igraph and libleidenalg. These are included as submodules and will be compiled automatically.

Automatic Build Script (Recommended)
Run the following script to build everything at once:

bash setup.sh

This script will:

Build and install igraph
Build and install libleidenalg
Compile the leiden_test executable
Manual Build Steps (Alternative)
If you prefer to build the dependencies manually, follow these steps:

2.1 Build and Install igraph

cd external/igraph
mkdir build
cd build
cmake .. -DCMAKE_INSTALL_PREFIX=../../install -DCMAKE_POSITION_INDEPENDENT_CODE=ON -DBUILD_SHARED_LIBS=ON
cmake --build . --target install
cd ../../../

2.2 Build and Install libleidenalg


cd external/libleidenalg
mkdir build
cd build
cmake .. -DCMAKE_PREFIX_PATH=../../install -DCMAKE_INSTALL_PREFIX=../../install
cmake --build . --target install
cd ../../../

2.3 Compile the Project

mkdir -p build
cd build
cmake ..
cmake --build .

### **3. Run the Leiden Algorithm**

Once everything is compiled, run the Leiden clustering test:


./build/leiden_test
Expected output:

./bin/leiden_clustering -t cpm -r 0.5 input.tsv output.tsv

Starting Leiden algorithm...
Leiden clustering complete.
