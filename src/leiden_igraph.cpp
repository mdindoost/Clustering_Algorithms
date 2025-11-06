// Leiden clustering in C++ using igraph (C API) + Apache Arrow/Parquet
// Reads edges from either TSV/CSV (two integer columns) or Parquet (two integer columns)


#include <algorithm>
#include <cctype>
#include <cerrno>
#include <charconv>
#include <cstdio>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>
#include <string_view>
#include <unordered_map>
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
    std::vector<Edge> edges; edges.reserve(1<<20);
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
          << " <input.{tsv|csv|parquet}> <output_dir> <dataset_name> <objective: modularity|cpm> <resolution> [--directed]\n"
          << "Usage (new): " << prog
          << " <input.{tsv|csv|parquet}> <output_dir> <objective: modularity|cpm> <resolution> [--directed]\n"
          << "Notes:\n"
          << "  - New form omits <dataset_name>; defaults to 'default_dataset'.\n"
          << "  - Graph is UNDIRECTED by default. Pass --directed to force (Leiden in igraph will error).\n"
          << "  - Output TSV: <output_dir>/<objective>/leiden_results.tsv (1-indexed community IDs)\n";
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
        std::vector<Edge> edges;
        if (has_ext(input_path, {".tsv", ".csv", ".txt"})) {
            std::cerr << "Reading TSV/CSV edges from: " << input_path << "\n";
            edges = read_tsv_edges(input_path);
        } else if (has_ext(input_path, {".parquet"})) {
            std::cerr << "Reading Parquet edges from: " << input_path << "\n";
            edges = read_parquet_edges(input_path);
        } else {
            throw std::runtime_error("Unsupported input extension: " + input_path.extension().string());
        }

        std::cerr << "Loaded " << edges.size() << " edges\n";

        igraph_t G; std::vector<long long> inv_map; build_graph_from_edges(edges, &G, directed, &inv_map);
        std::cerr << "Graph: " << (int)igraph_vcount(&G) << " vertices, " << (int)igraph_ecount(&G) << " edges\n";

        if (directed) {
            std::cerr << "Warning: Leiden in igraph only supports undirected graphs; directed run will fail.\n";
        }

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

        // Normalize to 0..C-1 (then we’ll output 1-indexed)
        IGRAPH_CHECK(igraph_reindex_membership(&membership, /*new_to_old=*/nullptr, &nb_clusters));

        // std::cout << "Leiden clustering complete. Found " << static_cast<long long>(nb_clusters) << " communities." << std::endl;
        std::cout << "Leiden clustering complete. Found " << static_cast<long long>(nb_clusters)
            << " communities. Quality=" << quality << std::endl;

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

        igraph_vector_int_destroy(&membership);
        igraph_destroy(&G);
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << "\n";
        return 2;
    }
}
