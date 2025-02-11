#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <sstream>
#include "run_leiden.h"

void print_usage(const char* program_name) {
    std::cout << "Usage: " << program_name << " [options] input_file output_file\n"
              << "Options:\n"
              << "  -t, --type TYPE       Modularity type (cpm or modularity)\n"
              << "  -r, --resolution VAL  Resolution parameter (default: 1.0)\n"
              << "Example:\n"
              << "  " << program_name << " -t cpm -r 0.5 input.tsv output.tsv\n";
}

bool read_graph(const std::string& filename, 
               std::vector<int64_t>& src, 
               std::vector<int64_t>& dst, 
               int64_t& max_node_id) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Error: Cannot open input file: " << filename << std::endl;
        return false;
    }

    std::string line;
    max_node_id = -1;
    while (std::getline(file, line)) {
        // Skip comment lines
        if (line.empty() || line[0] == '#') continue;

        std::istringstream iss(line);
        int64_t source, destination;
        if (!(iss >> source >> destination)) {
            std::cerr << "Error: Invalid line format in input file\n";
            return false;
        }

        src.push_back(source);
        dst.push_back(destination);
        max_node_id = std::max(max_node_id, std::max(source, destination));
    }

    if (src.empty()) {
        std::cerr << "Error: No valid edges found in input file\n";
        return false;
    }

    return true;
}

bool write_clusters(const std::string& filename, 
                   const int64_t communities[], 
                   int64_t num_nodes) {
    std::ofstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Error: Cannot open output file: " << filename << std::endl;
        return false;
    }

    for (int64_t i = 0; i < num_nodes; i++) {
        file << i << "\t" << communities[i] << "\n";
    }

    return true;
}

int main(int argc, char* argv[]) {
    std::string input_file;
    std::string output_file;
    std::string modularity_type = "modularity";
    double resolution = 1.0;

    // Parse command line arguments
    for (int i = 1; i < argc; i++) {
        std::string arg = argv[i];
        if (arg == "-h" || arg == "--help") {
            print_usage(argv[0]);
            return 0;
        } else if (arg == "-t" || arg == "--type") {
            if (i + 1 < argc) {
                modularity_type = argv[++i];
            }
        } else if (arg == "-r" || arg == "--resolution") {
            if (i + 1 < argc) {
                resolution = std::stod(argv[++i]);
            }
        } else if (input_file.empty()) {
            input_file = arg;
        } else if (output_file.empty()) {
            output_file = arg;
        }
    }

    if (input_file.empty() || output_file.empty()) {
        std::cerr << "Error: Input and output files must be specified\n";
        print_usage(argv[0]);
        return 1;
    }

    // Read graph
    std::vector<int64_t> src, dst;
    int64_t max_node_id;
    if (!read_graph(input_file, src, dst, max_node_id)) {
        return 1;
    }

    // Prepare for clustering
    int64_t num_edges = src.size();
    int64_t num_nodes = max_node_id + 1;
    std::vector<int64_t> communities(num_nodes);
    int64_t num_communities = 0;

    // Determine modularity type
    int64_t modularity_option;
    if (modularity_type == "cpm") {
        modularity_option = CPM;
    } else if (modularity_type == "modularity") {
        modularity_option = MODULARITY;
    } else {
        std::cerr << "Error: Invalid modularity type. Use 'cpm' or 'modularity'\n";
        return 1;
    }

    // Run Leiden algorithm
    run_leiden(src.data(), dst.data(), num_edges, num_nodes, 
              modularity_option, resolution, communities.data(), num_communities);

    // Write results
    if (!write_clusters(output_file, communities.data(), num_nodes)) {
        return 1;
    }

    std::cout << "Clustering complete. Results written to: " << output_file << std::endl;
    return 0;
}