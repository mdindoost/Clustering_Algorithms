# Clustering Algorithms with Leiden Community Detection

This repository provides multiple implementations of **graph clustering using the Leiden algorithm**, built upon `igraph`, `libleidenalg`, and (optionally) Apache Arrow for Parquet support.

**✨ Optimized for Large-Scale Graphs:** Binary CSR format (10-100x faster I/O), direct CSR loading (50% less memory), automatic edge deduplication, and detailed performance statistics.

Users can choose between:
1. **Classic Leiden (Python/libleidenalg + igraph)** — original build.
2. **New Leiden (C++ igraph + Arrow/Parquet)** — faster, standalone binary with TSV + Parquet + CSR + Binary CSR input.

---

## 🧭 Repository Structure
```
Clustering_Algorithms/
├── src/
│   ├── leiden_clustering.cpp    # Original implementation
│   ├── leiden_igraph.cpp        # New C++ Leiden (igraph + Arrow)
├── external/
│   ├── igraph/
│   ├── libleidenalg/
│   ├── arrow/
│   └── install/                 # Built dependencies (ignored in git)
├── scripts/
│   └── setup_dependencies.sh    # New unified build script
└── build/                       # CMake build directory
```

---

## ⚙️ Installation Guide

### **1. Clone the Repository**
```bash
git clone --recursive https://github.com/mdindoost/Clustering_Algorithms.git
cd Clustering_Algorithms
```

If you already cloned it without submodules:
```bash
git submodule update --init --recursive
```

---

### **2. Build and Install Dependencies**

#### Option A — Automatic (Recommended)
The unified script builds **igraph**, **libleidenalg**, and **Arrow/Parquet** inside `external/install`.

Make sure to load Intel and CMake on Wulver:
```bash
module load intel
module load CMake/3.31.3
bash scripts/setup_dependencies.sh
```

This script will:
- Build and install igraph  
- Build and install Apache Arrow + Parquet (C++)  
- Build and install libleidenalg  

---

#### Option B — Manual Build
If you prefer manual control:

**igraph**
```bash
cd external/igraph
mkdir build && cd build
cmake .. -DCMAKE_INSTALL_PREFIX=../../install -DCMAKE_POSITION_INDEPENDENT_CODE=ON -DBUILD_SHARED_LIBS=ON
cmake --build . --target install
cd ../../../
```

**libleidenalg**
```bash
cd external/libleidenalg
mkdir build && cd build
cmake .. -DCMAKE_PREFIX_PATH=../../install -DCMAKE_INSTALL_PREFIX=../../install
cmake --build . --target install
cd ../../../
```

**Apache Arrow + Parquet**
```bash
cd external/arrow/cpp
mkdir build && cd build
cmake .. -DCMAKE_INSTALL_PREFIX=../../install -DARROW_PARQUET=ON -DARROW_BUILD_SHARED=ON -DARROW_BUILD_STATIC=OFF -DARROW_BUILD_TESTS=OFF
cmake --build . --target install
cd ../../../..
```

---

### **3. Build the Project**

```bash
mkdir -p build && cd build
cmake .. -DCMAKE_PREFIX_PATH="../external/install"
cmake --build . --target leiden_igraph -j
```

You can still build the classic `leiden_clustering` binary:
```bash
cmake --build . --target leiden_clustering
```

---

## 🚀 Running Leiden

### **Performance Features (NEW!)**

**Binary CSR Format - 10-100x Faster I/O:**
```bash
# Convert once (one-time cost)
./build/convert_to_bcsr graph.tsv graph.bcsr

# Use for all experiments (much faster!)
./build/leiden_igraph graph.bcsr . modularity 1.0
./build/leiden_igraph graph.bcsr . cpm 0.5
```

**Automatic Optimizations:**
- ✅ Edge deduplication (TSV/Parquet)
- ✅ Direct CSR loading (50% less memory)
- ✅ Graph statistics output
- ✅ Performance timing
- ✅ Smart memory reservation

See [PERFORMANCE_GUIDE.md](PERFORMANCE_GUIDE.md) for details.

---

### **A. New C++ Leiden (igraph + Arrow/Parquet)**

**TSV Input**
```bash
./build/leiden_igraph edges.tsv . tmp_dataset_name cpm 0.5
```

**Parquet Input**
```bash
./build/leiden_igraph edges.parquet . my_dataset modularity 1.0
```

**CSR Input**
```bash
./build/leiden_igraph graph.csr . my_dataset cpm 0.5
```

**Binary CSR Input (Fastest - Recommended for Large Graphs!)**
```bash
# Convert once
./build/convert_to_bcsr edges.tsv graph.bcsr

# Use repeatedly (10-100x faster I/O)
./build/leiden_igraph graph.bcsr . modularity 1.0
```

Output:
```
./modularity/leiden_results.tsv   # Node-to-cluster assignments
./modularity/cluster_1.csr        # Subgraph for cluster 1
./modularity/cluster_2.csr        # Subgraph for cluster 2
...
```

**Format:**  
Each line → `<node_id>	<cluster_id>`

**CSR Format:**
The CSR (Compressed Sparse Row) format represents a graph efficiently:
- Line 1: Number of vertices (n)
- Line 2: Number of edges (nnz)
- Line 3: Row pointer array (n+1 space-separated integers)
- Line 4: Column index array (nnz space-separated integers)
- Line 5: Comment with original vertex IDs mapping

Example CSR file:
```
5
7
0 2 4 6 7 7
1 3 0 2 1 3 4
# Original vertex IDs: 0 1 2 3 4
```

**Cluster Subgraph Output:**
- Cluster subgraphs are numbered starting from 1 (e.g., `cluster_1.csr`, `cluster_2.csr`)
- This matches the 1-indexed cluster IDs in `leiden_results.tsv`
- Each subgraph file includes a comment line with the original vertex IDs for reference

**Supported Input Formats:**
- `.tsv`, `.csv`, `.txt` - Text edge lists (auto-deduplicated)
- `.parquet` - Apache Parquet (auto-detects src/dst columns)
- `.csr` - Text CSR format (direct loading, 50% less memory)
- `.bcsr` - **Binary CSR (10-100x faster I/O!)**

**Automatic Features:**
- ✅ Edge deduplication (TSV/Parquet)
- ✅ Self-loop removal
- ✅ Graph statistics output
- ✅ Performance timing
- ✅ CSR subgraph export
- ✅ Default mode: **undirected**

---

### **B. Original Leiden via libleidenalg**
```bash
./build/leiden_clustering -t cpm -r 0.5 input.tsv output.tsv
```
Example output:
```
Starting Leiden algorithm...
Leiden clustering complete.
```

---

## 🧠 Wulver Setup

Load the correct environment before building:
```bash
module load intel
module load imkl/2023.2.0
```

You can use the provided Makefile:
```bash
make clean
make
```

---

## 🧩 Arachne Users

If you encounter linker or include errors, extend your environment:
```bash
export LD_LIBRARY_PATH=/home/$USER/arkouda-njit/arachne/server/Clustering_Algorithms/external/install/lib64:$LD_LIBRARY_PATH
export LIBRARY_PATH=/home/$USER/arkouda-njit/arachne/server/Clustering_Algorithms/external/install/lib64:$LIBRARY_PATH
export CPATH=/home/$USER/arkouda-njit/arachne/server/Clustering_Algorithms/external/install/include:$CPATH
```

---

## 🧰 Run with Apptainer on Wulver

You can run everything in a containerized environment:
```bash
module load apptainer
apptainer pull cluster-algorithm.sif docker://spoiler2400/cluster-algorithm:1.0.0
apptainer exec cluster-algorithm.sif /app/build/leiden_clustering -t cpm -r 0.5 input.tsv output.tsv
```

---

## ✅ Summary of Choices

| Implementation | Language | Input Types | Output Format | Dependencies | Notes |
|----------------|-----------|--------------|----------------|--------------|-------|
| **leiden_clustering** | C++ / libleidenalg | TSV | TSV | igraph + libleidenalg | Original implementation |
| **leiden_igraph** | C++ (direct igraph + Arrow) | TSV / Parquet / CSR / **BCSR** | TSV + CSR subgraphs | igraph + Arrow/Parquet | **Optimized for large-scale**: Binary I/O, direct CSR loading, auto-dedup, stats |
| **convert_to_bcsr** | C++ utility | TSV / CSR | Binary CSR | Arrow | **10-100x faster I/O** for large graphs |

---

## 🧪 Quick Test Example

To verify your installation:

```bash
echo -e "0	1
1	2
2	3
3	4
4	0" > edges.tsv
# New (no dataset name)
./build/leiden_igraph edges.tsv . modularity 1.0

# Old (still works)
./build/leiden_igraph edges.tsv . my_dataset cpm 0.5
cat modularity/leiden_results.tsv
```

Expected output (cluster assignments will vary):
```
0   0
1   0
2   0
3   0
4   0
```

### CSR Format Test

To test CSR input format:

```bash
# Create a CSR format file
cat > graph.csr << 'EOF'
5
7
0 2 4 6 7 7
1 3 0 2 1 3 4
EOF

# Run clustering with CSR input
./build/leiden_igraph graph.csr . modularity 1.0

# Check the results
cat modularity/leiden_results.tsv
ls -la modularity/cluster_*.csr
```

This will produce:
- `modularity/leiden_results.tsv` - Node assignments to clusters
- `modularity/cluster_1.csr`, `modularity/cluster_2.csr`, etc. - Subgraphs for each cluster in CSR format

## Rebuild and run
```bash

cd build
cmake --build . --target leiden_igraph -j
cd ..
```
This confirms the new Leiden binary runs successfully on a small undirected graph.

---


*Maintained by Mohammad Dindoost — NJIT, Bader Lab.*