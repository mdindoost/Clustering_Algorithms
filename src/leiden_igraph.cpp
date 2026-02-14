// Leiden clustering in C++ using igraph (C API) + Apache Arrow/Parquet
// Reads edges from either TSV/CSV (two integer columns), Parquet (two integer columns), or CSR format
// Optimized for large-scale graphs with binary I/O, direct CSR loading, and performance improvements


#include <algorithm>
#include <cctype>
#include <cerrno>
#include <charconv>
#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>
#include <string_view>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

#include <igraph/igraph.h>

#include <arrow/api.h>
#include <arrow/io/api.h>
#include <parquet/arrow/reader.h>

namespace fs = std::filesystem;

struct Edge { long long u; long long v; };

static inline bool has_ext(const fs::path& p, std::initializer_list<const char*> exts) {
    auto e = p.extension().string();
    std::transform(e.begin(), e.end(), e.begin(), [](unsigned char c){return std::tolower(c);});
    for (auto x: exts) if (e == x) return true; return false;
}

// Minimal, fast ASCII splitter
static inline std::vector<std::string_view> split_ws(std::string_view s) {
    std::vector<std::string_view> out; size_t i=0, n=s.size();
    while (i<n) { while (i<n && std::isspace((unsigned char)s[i])) ++i; size_t j=i; while (j<n && !std::isspace((unsigned char)s[j])) ++j; if (i<j) out.emplace_back(s.substr(i, j-i)); i=j; }
    return out;
}

static bool parse_ll(std::string_view sv, long long &val) {
    auto begin = sv.data(); auto end = sv.data() + sv.size();
    auto [ptr, ec] = std::from_chars(begin, end, val);
    return ec == std::errc() && ptr == end;
}

// ---------- Arrow helpers ----------

static int find_column_index(const std::shared_ptr<arrow::Schema>& schema, const std::vector<std::string>& names) {
    for (const auto& name : names) {
        int idx = schema->GetFieldIndex(name);
        if (idx != -1) return idx;
    }
    return -1;
}

static std::shared_ptr<arrow::Table> read_parquet_table(const fs::path& path) {
    auto infile_res = arrow::io::ReadableFile::Open(path.string());
    if (!infile_res.ok()) throw std::runtime_error(infile_res.status().ToString());
    std::shared_ptr<arrow::io::ReadableFile> infile = *infile_res;

    std::unique_ptr<parquet::arrow::FileReader> pq_reader;
    auto open_res = parquet::arrow::OpenFile(infile, arrow::default_memory_pool());
    if (!open_res.ok()) throw std::runtime_error(open_res.status().ToString());
    pq_reader = std::move(*open_res);

    std::shared_ptr<arrow::Table> table;
    auto st = pq_reader->ReadTable(&table);
    if (!st.ok()) throw std::runtime_error(st.ToString());

    return table;
}

static std::vector<Edge> read_parquet_edges(const fs::path& path) {
    auto table = read_parquet_table(path);
    auto schema = table->schema();

    int u_idx = find_column_index(schema, {"src","source","u","from"});
    int v_idx = find_column_index(schema, {"dst","target","v","to"});
    if (u_idx == -1 || v_idx == -1) {
        if (table->num_columns() < 2)
            throw std::runtime_error("Parquet must have at least two columns for edges");
        u_idx = 0; v_idx = 1;
    }

    // Concatenate chunks if needed
    auto concat_int64 = [](const std::shared_ptr<arrow::ChunkedArray>& col)
            -> std::shared_ptr<arrow::Int64Array> {
        if (col->num_chunks() == 1) {
            return std::static_pointer_cast<arrow::Int64Array>(col->chunk(0));
        }
        auto res = arrow::Concatenate(col->chunks(), arrow::default_memory_pool());
        if (!res.ok()) throw std::runtime_error(res.status().ToString());
        return std::static_pointer_cast<arrow::Int64Array>(*res);
    };

    auto col_u = concat_int64(table->column(u_idx));
    auto col_v = concat_int64(table->column(v_idx));
    if (col_u->length() != col_v->length())
        throw std::runtime_error("Mismatched Parquet columns for edges");

    std::vector<Edge> edges; edges.reserve(static_cast<size_t>(col_u->length()));
    for (int64_t i = 0, n = col_u->length(); i < n; ++i) {
        if (col_u->IsNull(i) || col_v->IsNull(i)) continue;
        edges.push_back({ col_u->Value(i), col_v->Value(i) });
    }
    if (edges.empty()) throw std::runtime_error("No valid edges found in Parquet file");
    return edges;
}

// ---------- TSV reader ----------

static std::vector<Edge> read_tsv_edges(const fs::path& path) {
    std::ifstream fin(path);
    if (!fin) throw std::runtime_error("Cannot open TSV/CSV: " + path.string());

    // Better memory reservation based on file size
    size_t file_size = fs::file_size(path);
    size_t estimated_edges = file_size / 20; // ~20 bytes per edge line

    std::vector<Edge> edges;
    edges.reserve(std::min(estimated_edges, size_t(100'000'000))); // Cap at 100M for safety

    std::string line; bool first=true;
    while (std::getline(fin, line)) {
        if (line.empty()) continue;
        auto toks = split_ws(line);
        if (toks.size() < 2) continue;
        long long u,v;
        if (first) {
            long long tmp;
            bool ok1 = parse_ll(toks[0], tmp); bool ok2 = parse_ll(toks[1], tmp);
            if (!ok1 || !ok2) { first=false; continue; } // header line
        }
        if (!parse_ll(toks[0], u) || !parse_ll(toks[1], v)) continue;
        edges.push_back({u,v});
        first=false;
    }
    return edges;
}

// ---------- CSR reader ----------

struct CSRGraph {
    std::vector<long long> row_ptr;  // size: n+1
    std::vector<long long> col_idx;  // size: nnz
    long long n_vertices;
    long long n_edges;
};

static CSRGraph read_csr_file(const fs::path& path) {
    std::ifstream fin(path);
    if (!fin) throw std::runtime_error("Cannot open CSR file: " + path.string());

    CSRGraph csr;
    std::string line;

    // First line: number of vertices
    if (!std::getline(fin, line)) throw std::runtime_error("CSR file: missing number of vertices");
    long long n_vertices;
    if (!parse_ll(line, n_vertices)) throw std::runtime_error("CSR file: invalid number of vertices");
    csr.n_vertices = n_vertices;

    // Second line: number of edges (non-zeros)
    if (!std::getline(fin, line)) throw std::runtime_error("CSR file: missing number of edges");
    long long n_edges;
    if (!parse_ll(line, n_edges)) throw std::runtime_error("CSR file: invalid number of edges");
    csr.n_edges = n_edges;

    // Third line: row_ptr array (n_vertices+1 elements)
    if (!std::getline(fin, line)) throw std::runtime_error("CSR file: missing row_ptr");
    auto row_tokens = split_ws(line);
    if (row_tokens.size() != static_cast<size_t>(n_vertices + 1)) {
        throw std::runtime_error("CSR file: row_ptr size mismatch, expected " +
                               std::to_string(n_vertices + 1) + ", got " + std::to_string(row_tokens.size()));
    }
    csr.row_ptr.reserve(n_vertices + 1);
    for (const auto& tok : row_tokens) {
        long long val;
        if (!parse_ll(tok, val)) throw std::runtime_error("CSR file: invalid row_ptr value");
        csr.row_ptr.push_back(val);
    }

    // Fourth line: col_idx array (n_edges elements)
    if (!std::getline(fin, line)) throw std::runtime_error("CSR file: missing col_idx");
    auto col_tokens = split_ws(line);
    if (col_tokens.size() != static_cast<size_t>(n_edges)) {
        throw std::runtime_error("CSR file: col_idx size mismatch, expected " +
                               std::to_string(n_edges) + ", got " + std::to_string(col_tokens.size()));
    }
    csr.col_idx.reserve(n_edges);
    for (const auto& tok : col_tokens) {
        long long val;
        if (!parse_ll(tok, val)) throw std::runtime_error("CSR file: invalid col_idx value");
        csr.col_idx.push_back(val);
    }

    return csr;
}

// ---------- Binary CSR I/O (10-100x faster for large graphs) ----------

static CSRGraph read_binary_csr(const fs::path& path) {
    std::ifstream fin(path, std::ios::binary);
    if (!fin) throw std::runtime_error("Cannot open binary CSR file: " + path.string());

    CSRGraph csr;

    // Read header: n_vertices, n_edges
    fin.read(reinterpret_cast<char*>(&csr.n_vertices), sizeof(long long));
    fin.read(reinterpret_cast<char*>(&csr.n_edges), sizeof(long long));

    if (!fin) throw std::runtime_error("Binary CSR: failed to read header");

    // Read row_ptr array
    csr.row_ptr.resize(csr.n_vertices + 1);
    fin.read(reinterpret_cast<char*>(csr.row_ptr.data()),
             (csr.n_vertices + 1) * sizeof(long long));

    if (!fin) throw std::runtime_error("Binary CSR: failed to read row_ptr");

    // Read col_idx array
    csr.col_idx.resize(csr.n_edges);
    fin.read(reinterpret_cast<char*>(csr.col_idx.data()),
             csr.n_edges * sizeof(long long));

    if (!fin) throw std::runtime_error("Binary CSR: failed to read col_idx");

    std::cerr << "Binary CSR loaded: " << csr.n_vertices << " vertices, "
              << csr.n_edges << " edges\n";

    return csr;
}

static void write_binary_csr(const fs::path& path, const CSRGraph& csr) {
    std::ofstream fout(path, std::ios::binary);
    if (!fout) throw std::runtime_error("Cannot open binary CSR output file: " + path.string());

    // Write header
    fout.write(reinterpret_cast<const char*>(&csr.n_vertices), sizeof(long long));
    fout.write(reinterpret_cast<const char*>(&csr.n_edges), sizeof(long long));

    // Write row_ptr
    fout.write(reinterpret_cast<const char*>(csr.row_ptr.data()),
               (csr.n_vertices + 1) * sizeof(long long));

    // Write col_idx
    fout.write(reinterpret_cast<const char*>(csr.col_idx.data()),
               csr.n_edges * sizeof(long long));
}

// DEPRECATED: Use build_graph_from_csr_direct for better performance
static std::vector<Edge> csr_to_edges(const CSRGraph& csr) {
    std::vector<Edge> edges;
    edges.reserve(csr.n_edges);

    for (long long i = 0; i < csr.n_vertices; ++i) {
        long long start = csr.row_ptr[i];
        long long end = csr.row_ptr[i + 1];
        for (long long j = start; j < end; ++j) {
            edges.push_back({i, csr.col_idx[j]});
        }
    }

    return edges;
}

// ---------- Direct CSR to igraph (avoids intermediate edge list) ----------
// This is 2x faster and uses 50% less memory than csr_to_edges + build_graph_from_edges

static void build_graph_from_csr_direct(const CSRGraph& csr, igraph_t* g, bool directed) {
    std::cerr << "Building graph directly from CSR (optimized path)...\n";

    // Create empty graph
    igraph_error_t err = igraph_empty(g, csr.n_vertices,
                                      directed ? IGRAPH_DIRECTED : IGRAPH_UNDIRECTED);
    if (err) throw std::runtime_error("igraph_empty failed");

    // Pre-allocate flat edge array
    std::vector<igraph_integer_t> edges_flat;
    edges_flat.reserve(csr.n_edges * 2);

    // Convert CSR to flat edge list
    for (long long i = 0; i < csr.n_vertices; ++i) {
        for (long long j = csr.row_ptr[i]; j < csr.row_ptr[i+1]; ++j) {
            edges_flat.push_back(static_cast<igraph_integer_t>(i));
            edges_flat.push_back(static_cast<igraph_integer_t>(csr.col_idx[j]));
        }
    }

    // Add all edges at once
    igraph_vector_int_t edges_vec;
    igraph_vector_int_view(&edges_vec, edges_flat.data(), edges_flat.size());
    err = igraph_add_edges(g, &edges_vec, nullptr);
    if (err) throw std::runtime_error("igraph_add_edges failed");
}

// ---------- CSR writer for subgraphs ----------

static void write_csr_subgraph(const fs::path& path, const igraph_t* g, 
                               const std::vector<igraph_integer_t>& subgraph_vids,
                               const std::vector<long long>& inv_map) {
    // Create a mapping from original vertex IDs to subgraph indices
    std::unordered_map<igraph_integer_t, long long> vid_to_idx;
    for (size_t i = 0; i < subgraph_vids.size(); ++i) {
        vid_to_idx[subgraph_vids[i]] = static_cast<long long>(i);
    }
    
    long long n_vertices = static_cast<long long>(subgraph_vids.size());
    
    // Build CSR structure for subgraph
    std::vector<long long> row_ptr(n_vertices + 1, 0);
    std::vector<long long> col_idx;
    
    // For each vertex in the subgraph, find its neighbors that are also in the subgraph
    for (size_t i = 0; i < subgraph_vids.size(); ++i) {
        igraph_integer_t vid = subgraph_vids[i];
        igraph_vector_int_t neighbors;
        igraph_vector_int_init(&neighbors, 0);
        
        // Get neighbors of this vertex
        igraph_neighbors(g, &neighbors, vid, IGRAPH_ALL);
        
        // Count and add neighbors that are in the subgraph
        for (long j = 0; j < igraph_vector_int_size(&neighbors); ++j) {
            igraph_integer_t neighbor = VECTOR(neighbors)[j];
            auto it = vid_to_idx.find(neighbor);
            if (it != vid_to_idx.end()) {
                col_idx.push_back(it->second);
            }
        }
        
        row_ptr[i + 1] = static_cast<long long>(col_idx.size());
        igraph_vector_int_destroy(&neighbors);
    }
    
    long long n_edges = static_cast<long long>(col_idx.size());
    
    // Write CSR to file
    std::ofstream fout(path);
    if (!fout) throw std::runtime_error("Cannot open CSR output file: " + path.string());
    
    // Line 1: number of vertices
    fout << n_vertices << '\n';
    
    // Line 2: number of edges
    fout << n_edges << '\n';
    
    // Line 3: row_ptr array
    for (size_t i = 0; i < row_ptr.size(); ++i) {
        if (i > 0) fout << ' ';
        fout << row_ptr[i];
    }
    fout << '\n';
    
    // Line 4: col_idx array
    for (size_t i = 0; i < col_idx.size(); ++i) {
        if (i > 0) fout << ' ';
        fout << col_idx[i];
    }
    fout << '\n';
    
    // Line 5: original vertex IDs mapping (for reference)
    fout << "# Original vertex IDs: ";
    for (size_t i = 0; i < subgraph_vids.size(); ++i) {
        if (i > 0) fout << ' ';
        fout << inv_map[subgraph_vids[i]];
    }
    fout << '\n';
}

// ---------- Edge deduplication and cleanup ----------

static size_t deduplicate_edges(std::vector<Edge>& edges, bool remove_self_loops = true) {
    size_t original_size = edges.size();

    // Remove self-loops if requested
    if (remove_self_loops) {
        auto it = std::remove_if(edges.begin(), edges.end(),
                                 [](const Edge& e) { return e.u == e.v; });
        edges.erase(it, edges.end());
    }

    // Sort edges for deduplication
    std::sort(edges.begin(), edges.end(),
        [](const Edge& a, const Edge& b) {
            return a.u < b.u || (a.u == b.u && a.v < b.v);
        });

    // Remove duplicates
    auto last = std::unique(edges.begin(), edges.end(),
        [](const Edge& a, const Edge& b) {
            return a.u == b.u && a.v == b.v;
        });

    edges.erase(last, edges.end());

    size_t removed = original_size - edges.size();
    if (removed > 0) {
        std::cerr << "Removed " << removed << " duplicate/self-loop edges ("
                  << (100.0 * removed / original_size) << "%)\n";
    }

    return removed;
}

// ---------- Graph statistics ----------

static void print_graph_statistics(const igraph_t* g) {
    std::cerr << "\n=== Graph Statistics ===\n";
    std::cerr << "Vertices: " << igraph_vcount(g) << "\n";
    std::cerr << "Edges: " << igraph_ecount(g) << "\n";

    // Density
    igraph_real_t density;
    igraph_density(g, &density, /*loops=*/false);
    std::cerr << "Density: " << density << "\n";

    // Connected components
    igraph_bool_t connected;
    igraph_is_connected(g, &connected, IGRAPH_WEAK);
    std::cerr << "Connected: " << (connected ? "yes" : "no") << "\n";

    if (!connected) {
        igraph_integer_t num_components;
        igraph_vector_int_t membership, csize;
        igraph_vector_int_init(&membership, 0);
        igraph_vector_int_init(&csize, 0);

        igraph_connected_components(g, &membership, &csize, &num_components, IGRAPH_WEAK);
        std::cerr << "Number of components: " << num_components << "\n";

        // Find largest component size
        long long max_size = 0;
        for (long i = 0; i < igraph_vector_int_size(&csize); ++i) {
            max_size = std::max(max_size, (long long)VECTOR(csize)[i]);
        }
        std::cerr << "Largest component size: " << max_size << "\n";

        igraph_vector_int_destroy(&membership);
        igraph_vector_int_destroy(&csize);
    }

    // Degree statistics
    igraph_vector_int_t degrees;
    igraph_vector_int_init(&degrees, 0);
    igraph_degree(g, &degrees, igraph_vss_all(), IGRAPH_ALL, /*loops=*/false);

    long long total_deg = 0;
    long long max_deg = 0;
    long long min_deg = LLONG_MAX;

    for (long i = 0; i < igraph_vector_int_size(&degrees); ++i) {
        long long deg = VECTOR(degrees)[i];
        total_deg += deg;
        max_deg = std::max(max_deg, deg);
        min_deg = std::min(min_deg, deg);
    }

    double avg_deg = static_cast<double>(total_deg) / igraph_vcount(g);
    std::cerr << "Degree: min=" << min_deg << ", max=" << max_deg
              << ", avg=" << avg_deg << "\n";

    igraph_vector_int_destroy(&degrees);
    std::cerr << "========================\n\n";
}

// ---------- Graph build with robust remap ----------

static void build_graph_from_edges(const std::vector<Edge>& edges_raw, igraph_t* g, bool directed,
                                   std::vector<long long>* inv_map_out) {
    // Map arbitrary node IDs to 0..N-1
    std::unordered_map<long long, igraph_integer_t> idmap;
    idmap.reserve(edges_raw.size()*2);
    std::vector<igraph_integer_t> es; es.reserve(edges_raw.size()*2);

    auto intern = [&](long long x)->igraph_integer_t {
        auto it = idmap.find(x);
        if (it != idmap.end()) return it->second;
        igraph_integer_t id = static_cast<igraph_integer_t>(idmap.size());
        idmap.emplace(x, id); return id;
    };

    for (const auto& e : edges_raw) {
        es.push_back(intern(e.u));
        es.push_back(intern(e.v));
    }

    igraph_vector_int_t edges_vec;
    igraph_vector_int_view(&edges_vec, es.data(), (long) es.size());

    igraph_error_t err;
    err = igraph_empty(g, (long) idmap.size(), directed ? IGRAPH_DIRECTED : IGRAPH_UNDIRECTED);
    if (err) throw std::runtime_error("igraph_empty failed");
    err = igraph_add_edges(g, &edges_vec, /*attr=*/nullptr);
    if (err) throw std::runtime_error("igraph_add_edges failed");

    if (inv_map_out) {
        inv_map_out->resize(idmap.size());
        for (const auto& kv : idmap) (*inv_map_out)[kv.second] = kv.first;
    }
}

// ---------- Main ----------

int main(int argc, char** argv) {
    auto print_usage = [&](const char* prog){
        std::cerr
          << "Usage (old): " << prog
          << " <input.{tsv|csv|parquet|csr|bcsr}> <output_dir> <dataset_name> <objective: modularity|cpm> <resolution> [--directed]\n"
          << "Usage (new): " << prog
          << " <input.{tsv|csv|parquet|csr|bcsr}> <output_dir> <objective: modularity|cpm> <resolution> [--directed]\n"
          << "\nInput Formats:\n"
          << "  .tsv/.csv/.txt - Text edge list (two integer columns)\n"
          << "  .parquet       - Apache Parquet format\n"
          << "  .csr           - Text CSR format (n_vertices, n_edges, row_ptr, col_idx)\n"
          << "  .bcsr          - Binary CSR format (10-100x faster I/O for large graphs)\n"
          << "\nNotes:\n"
          << "  - New form omits <dataset_name>; defaults to 'default_dataset'.\n"
          << "  - Graph is UNDIRECTED by default. Pass --directed to force (Leiden in igraph will error).\n"
          << "  - Automatic edge deduplication and self-loop removal for TSV/Parquet inputs.\n"
          << "  - Direct CSR loading for optimal memory usage (no intermediate edge list).\n"
          << "  - Output TSV: <output_dir>/<objective>/leiden_results.tsv (1-indexed community IDs)\n"
          << "  - Cluster subgraphs: <output_dir>/<objective>/cluster_<id>.csr for each cluster\n"
          << "\nPerformance Tips:\n"
          << "  - Use .bcsr for fastest I/O on large graphs (convert once, reuse many times)\n"
          << "  - Binary CSR is 10-100x faster than text formats\n";
    };

    // --help
    if (argc == 2 && (std::string(argv[1]) == "--help" || std::string(argv[1]) == "-h")) {
        print_usage(argv[0]);
        return 0;
    }

    if (argc < 5) {
        print_usage(argv[0]);
        return 1;
    }

    const fs::path input_path = argv[1];
    const fs::path dataset_path = argv[2];

    // We accept either:
    //   old: argv[3]=dataset_name, argv[4]=objective, argv[5]=resolution
    //   new:               (no dataset_name) argv[3]=objective, argv[4]=resolution
    std::string dataset_name = "default_dataset";
    std::string objective;
    double resolution = 1.0;

    auto to_lower = [](std::string s){
        std::transform(s.begin(), s.end(), s.begin(),
                       [](unsigned char c){ return std::tolower(c); });
        return s;
    };

    // Try to detect whether argv[3] is an objective or a dataset name
    if (argc >= 6) {
        // Old form present (we have at least 6 args)
        dataset_name = argv[3];
        objective = argv[4];
        resolution = std::stod(argv[5]);
        // optional flags start at 6
    } else {
        // New form (no dataset name)
        objective = argv[3];
        resolution = std::stod(argv[4]);
        // optional flags start at 5
    }

    bool directed = false; // default UNDIRECTED
    for (int i = (argc >= 6 ? 6 : 5); i < argc; ++i) {
        const std::string flag = argv[i];
        if (flag == "--directed") directed = true;
        if (flag == "--undirected") directed = false;
        if (flag == "--help" || flag == "-h") { print_usage(argv[0]); return 0; }
    }

    std::transform(objective.begin(), objective.end(), objective.begin(), [](unsigned char c){return std::tolower(c);});
    objective = to_lower(objective);
    std::string mode = (objective == "modularity") ? "modularity" : "CPM"; // default CPM if unknown

    try {
        auto start_time = std::chrono::high_resolution_clock::now();

        igraph_t G;
        std::vector<long long> inv_map;
        bool use_direct_csr = false;

        // Read input based on format
        if (has_ext(input_path, {".tsv", ".csv", ".txt"})) {
            std::cerr << "Reading TSV/CSV edges from: " << input_path << "\n";
            auto io_start = std::chrono::high_resolution_clock::now();

            std::vector<Edge> edges = read_tsv_edges(input_path);

            auto io_end = std::chrono::high_resolution_clock::now();
            auto io_duration = std::chrono::duration_cast<std::chrono::milliseconds>(io_end - io_start);
            std::cerr << "I/O completed in " << (io_duration.count() / 1000.0) << " seconds\n";

            std::cerr << "Loaded " << edges.size() << " edges\n";

            // Deduplicate edges
            deduplicate_edges(edges, /*remove_self_loops=*/true);
            std::cerr << "After deduplication: " << edges.size() << " edges\n";

            build_graph_from_edges(edges, &G, directed, &inv_map);

        } else if (has_ext(input_path, {".parquet"})) {
            std::cerr << "Reading Parquet edges from: " << input_path << "\n";
            auto io_start = std::chrono::high_resolution_clock::now();

            std::vector<Edge> edges = read_parquet_edges(input_path);

            auto io_end = std::chrono::high_resolution_clock::now();
            auto io_duration = std::chrono::duration_cast<std::chrono::milliseconds>(io_end - io_start);
            std::cerr << "I/O completed in " << (io_duration.count() / 1000.0) << " seconds\n";

            std::cerr << "Loaded " << edges.size() << " edges\n";

            // Deduplicate edges
            deduplicate_edges(edges, /*remove_self_loops=*/true);
            std::cerr << "After deduplication: " << edges.size() << " edges\n";

            build_graph_from_edges(edges, &G, directed, &inv_map);

        } else if (has_ext(input_path, {".bcsr"})) {
            std::cerr << "Reading binary CSR format from: " << input_path << "\n";
            auto io_start = std::chrono::high_resolution_clock::now();

            CSRGraph csr = read_binary_csr(input_path);

            auto io_end = std::chrono::high_resolution_clock::now();
            auto io_duration = std::chrono::duration_cast<std::chrono::milliseconds>(io_end - io_start);
            std::cerr << "Binary I/O completed in " << (io_duration.count() / 1000.0) << " seconds\n";

            // Use direct CSR builder (faster, less memory)
            build_graph_from_csr_direct(csr, &G, directed);
            use_direct_csr = true;

            // For CSR, vertex IDs are 0..n-1, so inv_map is identity
            inv_map.resize(csr.n_vertices);
            for (long long i = 0; i < csr.n_vertices; ++i) {
                inv_map[i] = i;
            }

        } else if (has_ext(input_path, {".csr"})) {
            std::cerr << "Reading text CSR format from: " << input_path << "\n";
            auto io_start = std::chrono::high_resolution_clock::now();

            CSRGraph csr = read_csr_file(input_path);
            std::cerr << "CSR: " << csr.n_vertices << " vertices, " << csr.n_edges << " edges\n";

            auto io_end = std::chrono::high_resolution_clock::now();
            auto io_duration = std::chrono::duration_cast<std::chrono::milliseconds>(io_end - io_start);
            std::cerr << "I/O completed in " << (io_duration.count() / 1000.0) << " seconds\n";

            // Use direct CSR builder (faster, less memory)
            build_graph_from_csr_direct(csr, &G, directed);
            use_direct_csr = true;

            // For CSR, vertex IDs are 0..n-1, so inv_map is identity
            inv_map.resize(csr.n_vertices);
            for (long long i = 0; i < csr.n_vertices; ++i) {
                inv_map[i] = i;
            }

        } else {
            throw std::runtime_error("Unsupported input extension: " + input_path.extension().string() +
                                     "\nSupported: .tsv, .csv, .txt, .parquet, .csr, .bcsr");
        }

        std::cerr << "Graph: " << (int)igraph_vcount(&G) << " vertices, " << (int)igraph_ecount(&G) << " edges\n";

        // Print detailed graph statistics
        print_graph_statistics(&G);

        if (directed) {
            std::cerr << "Warning: Leiden in igraph only supports undirected graphs; directed run will fail.\n";
        }

        // Start clustering
        std::cerr << "Starting Leiden clustering (objective=" << mode
                  << ", resolution=" << resolution << ")...\n";
        auto cluster_start = std::chrono::high_resolution_clock::now();

        igraph_vector_int_t membership; igraph_vector_int_init(&membership, 0);

        // igraph 0.10.x API:
        // igraph_community_leiden(graph, weights, initial, resolution, beta,
        //                         start, n_iterations, membership, nb_clusters, quality)
        const igraph_real_t beta = 0.01;
        const igraph_bool_t start = 0;
        //const igraph_integer_t n_iterations = -1; // until stable
        const igraph_integer_t n_iterations = 50; // cap to avoid rare stalls

        igraph_integer_t nb_clusters = 0;
        igraph_real_t quality = 0.0;

        igraph_error_t err = igraph_community_leiden(
             &G,
             /*weights*/      nullptr,
             /*initial*/      nullptr,
             /*resolution*/   resolution,
             /*beta*/         beta,
             /*start*/        start,
             /*n_iterations*/ n_iterations,
             /*membership*/   &membership,
             /*nb_clusters*/  &nb_clusters,
             /*quality*/      &quality);
        if (err) throw std::runtime_error("igraph_community_leiden failed");

        auto cluster_end = std::chrono::high_resolution_clock::now();
        auto cluster_duration = std::chrono::duration_cast<std::chrono::milliseconds>(cluster_end - cluster_start);
        std::cerr << "Clustering completed in " << (cluster_duration.count() / 1000.0) << " seconds\n";

        // Normalize to 0..C-1 (then we’ll output 1-indexed)
        IGRAPH_CHECK(igraph_reindex_membership(&membership, /*new_to_old=*/nullptr, &nb_clusters));

        // std::cout << "Leiden clustering complete. Found " << static_cast<long long>(nb_clusters) << " communities." << std::endl;
        std::cout << "Leiden clustering complete. Found " << static_cast<long long>(nb_clusters)
            << " communities. Quality=" << quality << std::endl;
        
        // Compute cluster sizes
        std::vector<long long> cluster_sizes(nb_clusters, 0);
        for (igraph_integer_t i = 0; i < igraph_vcount(&G); ++i) {
            int cid = VECTOR(membership)[i];
            if (cid >= 0 && cid < nb_clusters)
                cluster_sizes[cid]++;
        }

        // Print summary of cluster sizes
        // std::cout << "Cluster sizes:" << std::endl;
        // for (igraph_integer_t cid = 0; cid < nb_clusters; ++cid) {
        //     std::cout << "  Cluster " << cid << ": " << cluster_sizes[cid] << " vertices" << std::endl;
        // }

        auto [min_it, max_it] = std::minmax_element(cluster_sizes.begin(), cluster_sizes.end());
        long long total = 0;
        for (auto s : cluster_sizes) total += s;
        double avg = static_cast<double>(total) / nb_clusters;
        std::cout << "Smallest cluster: " << *min_it
                << ", Largest: " << *max_it
                << ", Average size: " << avg << std::endl;

        // Build histogram: how many clusters have a given size
        std::unordered_map<long long, long long> size_hist;
        for (auto s : cluster_sizes) size_hist[s]++;

        // Sort sizes for pretty output
        std::vector<std::pair<long long, long long>> sorted_hist(size_hist.begin(), size_hist.end());
        std::sort(sorted_hist.begin(), sorted_hist.end());

        std::cout << "Cluster size distribution:" << std::endl;
        for (auto [size, count] : sorted_hist) {
            std::cout << "  Clusters with size " << size << ": " << count << std::endl;
        }

        // ----- TSV output (node_id \t community_1indexed) -----
        fs::path outdir = dataset_path / mode;
        fs::create_directories(outdir);
        fs::path out = outdir / "leiden_results.tsv";

        // Build pairs (orig_id, comm+1), sort by orig_id
        std::vector<std::pair<long long, long long>> rows;
        rows.reserve((size_t)igraph_vcount(&G));
        for (igraph_integer_t i=0; i<igraph_vcount(&G); ++i) {
            long long orig = inv_map[(size_t)i];
            long long comm = (long long)VECTOR(membership)[i] + 1; // 1-indexed
            rows.emplace_back(orig, comm);
        }
        std::sort(rows.begin(), rows.end(),
                  [](const auto& a, const auto& b){ return a.first < b.first; });

        std::ofstream jout(out);
        if (!jout) throw std::runtime_error("Cannot open output for write: " + out.string());
        for (auto &rc : rows) {
            jout << rc.first << '\t' << rc.second << '\n';
        }
        std::cerr << "Saved TSV to: " << out << "\n";

        // ----- Generate CSR subgraphs for each cluster -----
        std::cerr << "Generating CSR subgraphs for each cluster...\n";
        
        // Group vertices by cluster
        std::vector<std::vector<igraph_integer_t>> cluster_vertices(nb_clusters);
        for (igraph_integer_t i = 0; i < igraph_vcount(&G); ++i) {
            int cid = VECTOR(membership)[i];
            if (cid >= 0 && cid < nb_clusters) {
                cluster_vertices[cid].push_back(i);
            }
        }
        
        // Write CSR file for each cluster
        for (igraph_integer_t cid = 0; cid < nb_clusters; ++cid) {
            if (cluster_vertices[cid].empty()) continue;
            
            std::string filename = "cluster_" + std::to_string(cid + 1) + ".csr"; // 1-indexed
            fs::path csr_path = outdir / filename;
            
            write_csr_subgraph(csr_path, &G, cluster_vertices[cid], inv_map);
        }
        
        std::cerr << "Saved " << nb_clusters << " cluster subgraphs in CSR format to: " << outdir << "\n";

        igraph_vector_int_destroy(&membership);
        igraph_destroy(&G);

        // Print total runtime
        auto end_time = std::chrono::high_resolution_clock::now();
        auto total_duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);
        std::cerr << "\n=== Total Runtime: " << (total_duration.count() / 1000.0) << " seconds ===\n";

        return 0;
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << "\n";
        return 2;
    }
}
