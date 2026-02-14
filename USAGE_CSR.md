# CSR Format Usage Guide

This document describes how to use CSR (Compressed Sparse Row) format with the Leiden clustering algorithm.

## CSR Format Specification

The CSR format is a space-efficient way to represent sparse graphs. Each CSR file contains:

```
Line 1: n_vertices (number of vertices)
Line 2: n_edges (number of edges/non-zeros)
Line 3: row_ptr (n_vertices+1 space-separated integers)
Line 4: col_idx (n_edges space-separated integers)
Line 5: # Comment with original vertex IDs (for subgraphs)
```

### Example CSR File

```
5
10
0 2 4 6 8 10
1 2 0 3 1 4 2 0 3 4
```

This represents a graph with:
- 5 vertices (0, 1, 2, 3, 4)
- 10 edges
- Vertex 0 has edges to vertices at col_idx[0:2] = [1, 2]
- Vertex 1 has edges to vertices at col_idx[2:4] = [0, 3]
- Vertex 2 has edges to vertices at col_idx[4:6] = [1, 4]
- Vertex 3 has edges to vertices at col_idx[6:8] = [2, 0]
- Vertex 4 has edges to vertices at col_idx[8:10] = [3, 4]

## Input: Using CSR Files

To run Leiden clustering with a CSR input file:

```bash
./build/leiden_igraph input.csr . cpm 0.5
```

Or with the modularity objective:

```bash
./build/leiden_igraph input.csr . modularity 1.0
```

## Output: Cluster Subgraphs

After clustering, the algorithm produces:

1. **leiden_results.tsv** - Node-to-cluster assignments
2. **cluster_N.csr** - Subgraph for each cluster in CSR format

### Cluster Subgraph Format

Each cluster subgraph file includes:
- Renumbered vertex IDs (0-indexed within the cluster)
- Only edges between vertices within the same cluster
- A comment line mapping subgraph indices to original vertex IDs

## Complete Workflow Example

```bash
# 1. Create or prepare your graph in CSR format
cat > mygraph.csr << 'EOF'
5
10
0 2 4 6 8 10
1 2 0 3 1 4 2 0 3 4
EOF

# 2. Run Leiden clustering
./build/leiden_igraph mygraph.csr . modularity 1.0

# 3. Check results
cat modularity/leiden_results.tsv

# 4. Examine cluster subgraphs
ls -la modularity/cluster_*.csr
```

## Notes

- Cluster IDs are 1-indexed (matching leiden_results.tsv)
- Subgraph vertex IDs are always 0-indexed
- The mapping comment line helps trace vertices back to the original graph
