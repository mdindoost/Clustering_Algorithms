// Utility to convert graph formats to binary CSR for faster I/O
// Usage: convert_to_bcsr <input.{tsv|csv|parquet|csr}> <output.bcsr>

#include <algorithm>
#include <cctype>
#include <charconv>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

#include <arrow/api.h>
#include <arrow/io/api.h>
#include <parquet/arrow/reader.h>

namespace fs = std::filesystem;

struct Edge { long long u; long long v; };

struct CSRGraph {
    std::vector<long long> row_ptr;
    std::vector<long long> col_idx;
    long long n_vertices;
    long long n_edges;
};

static inline bool has_ext(const fs::path& p, std::initializer_list<const char*> exts) {
    auto e = p.extension().string();
    std::transform(e.begin(), e.end(), e.begin(), [](unsigned char c){return std::tolower(c);});
    for (auto x: exts) if (e == x) return true; return false;
}

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

// Read TSV edges
static std::vector<Edge> read_tsv_edges(const fs::path& path) {
    std::ifstream fin(path);
    if (!fin) throw std::runtime_error("Cannot open TSV/CSV: " + path.string());

    std::vector<Edge> edges;
    edges.reserve(1'000'000);

    std::string line;
    bool first = true;

    while (std::getline(fin, line)) {
        if (line.empty()) continue;
        auto toks = split_ws(line);
        if (toks.size() < 2) continue;

        long long u, v;
        if (first) {
            long long tmp;
            bool ok1 = parse_ll(toks[0], tmp);
            bool ok2 = parse_ll(toks[1], tmp);
            if (!ok1 || !ok2) { first = false; continue; }
        }

        if (!parse_ll(toks[0], u) || !parse_ll(toks[1], v)) continue;
        edges.push_back({u, v});
        first = false;
    }

    return edges;
}

// Read text CSR
static CSRGraph read_csr_file(const fs::path& path) {
    std::ifstream fin(path);
    if (!fin) throw std::runtime_error("Cannot open CSR file: " + path.string());

    CSRGraph csr;
    std::string line;

    if (!std::getline(fin, line)) throw std::runtime_error("CSR: missing n_vertices");
    if (!parse_ll(line, csr.n_vertices)) throw std::runtime_error("CSR: invalid n_vertices");

    if (!std::getline(fin, line)) throw std::runtime_error("CSR: missing n_edges");
    if (!parse_ll(line, csr.n_edges)) throw std::runtime_error("CSR: invalid n_edges");

    if (!std::getline(fin, line)) throw std::runtime_error("CSR: missing row_ptr");
    auto row_tokens = split_ws(line);
    if (row_tokens.size() != static_cast<size_t>(csr.n_vertices + 1)) {
        throw std::runtime_error("CSR: row_ptr size mismatch");
    }

    csr.row_ptr.reserve(csr.n_vertices + 1);
    for (const auto& tok : row_tokens) {
        long long val;
        if (!parse_ll(tok, val)) throw std::runtime_error("CSR: invalid row_ptr value");
        csr.row_ptr.push_back(val);
    }

    if (!std::getline(fin, line)) throw std::runtime_error("CSR: missing col_idx");
    auto col_tokens = split_ws(line);
    if (col_tokens.size() != static_cast<size_t>(csr.n_edges)) {
        throw std::runtime_error("CSR: col_idx size mismatch");
    }

    csr.col_idx.reserve(csr.n_edges);
    for (const auto& tok : col_tokens) {
        long long val;
        if (!parse_ll(tok, val)) throw std::runtime_error("CSR: invalid col_idx value");
        csr.col_idx.push_back(val);
    }

    return csr;
}

// Convert edge list to CSR
static CSRGraph edges_to_csr(const std::vector<Edge>& edges) {
    // Find vertex IDs and remap to 0..n-1
    std::unordered_map<long long, long long> id_map;

    for (const auto& e : edges) {
        if (id_map.find(e.u) == id_map.end()) {
            long long new_id = id_map.size();
            id_map[e.u] = new_id;
        }
        if (id_map.find(e.v) == id_map.end()) {
            long long new_id = id_map.size();
            id_map[e.v] = new_id;
        }
    }

    CSRGraph csr;
    csr.n_vertices = id_map.size();
    csr.n_edges = edges.size();

    // Build adjacency lists
    std::vector<std::vector<long long>> adj(csr.n_vertices);
    for (const auto& e : edges) {
        long long u = id_map[e.u];
        long long v = id_map[e.v];
        adj[u].push_back(v);
    }

    // Convert to CSR
    csr.row_ptr.resize(csr.n_vertices + 1);
    csr.row_ptr[0] = 0;

    for (long long i = 0; i < csr.n_vertices; ++i) {
        csr.row_ptr[i + 1] = csr.row_ptr[i] + adj[i].size();
        for (long long neighbor : adj[i]) {
            csr.col_idx.push_back(neighbor);
        }
    }

    return csr;
}

// Write binary CSR
static void write_binary_csr(const fs::path& path, const CSRGraph& csr) {
    std::ofstream fout(path, std::ios::binary);
    if (!fout) throw std::runtime_error("Cannot open output file: " + path.string());

    fout.write(reinterpret_cast<const char*>(&csr.n_vertices), sizeof(long long));
    fout.write(reinterpret_cast<const char*>(&csr.n_edges), sizeof(long long));

    fout.write(reinterpret_cast<const char*>(csr.row_ptr.data()),
               (csr.n_vertices + 1) * sizeof(long long));

    fout.write(reinterpret_cast<const char*>(csr.col_idx.data()),
               csr.n_edges * sizeof(long long));

    std::cout << "Written binary CSR: " << csr.n_vertices << " vertices, "
              << csr.n_edges << " edges\n";
}

int main(int argc, char** argv) {
    if (argc != 3) {
        std::cerr << "Usage: " << argv[0] << " <input.{tsv|csv|csr}> <output.bcsr>\n";
        std::cerr << "\nConverts graph to binary CSR format for 10-100x faster I/O.\n";
        std::cerr << "Binary CSR is ideal for large graphs that you'll cluster multiple times.\n";
        return 1;
    }

    fs::path input_path = argv[1];
    fs::path output_path = argv[2];

    try {
        CSRGraph csr;

        if (has_ext(input_path, {".tsv", ".csv", ".txt"})) {
            std::cout << "Reading TSV/CSV from: " << input_path << "\n";
            auto edges = read_tsv_edges(input_path);
            std::cout << "Loaded " << edges.size() << " edges\n";

            std::cout << "Converting to CSR...\n";
            csr = edges_to_csr(edges);

        } else if (has_ext(input_path, {".csr"})) {
            std::cout << "Reading text CSR from: " << input_path << "\n";
            csr = read_csr_file(input_path);

        } else {
            throw std::runtime_error("Unsupported input format. Use .tsv, .csv, or .csr");
        }

        std::cout << "Writing binary CSR to: " << output_path << "\n";
        write_binary_csr(output_path, csr);

        // Calculate space savings
        size_t input_size = fs::file_size(input_path);
        size_t output_size = fs::file_size(output_path);
        double ratio = static_cast<double>(input_size) / output_size;

        std::cout << "\nFile sizes:\n";
        std::cout << "  Input:  " << (input_size / 1024 / 1024) << " MB\n";
        std::cout << "  Output: " << (output_size / 1024 / 1024) << " MB\n";
        std::cout << "  Compression: " << ratio << "x\n";

        return 0;

    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << "\n";
        return 2;
    }
}
