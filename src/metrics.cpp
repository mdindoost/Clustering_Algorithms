// metrics.cpp
#include "metrics.h"

double computeARI(const std::vector<int64_t>& labels_true, const std::vector<int64_t>& labels_pred) {
    if (labels_true.size() != labels_pred.size()) {
        throw std::invalid_argument("Label vectors must have the same length.");
    }

    size_t n = labels_true.size();

    // Mapping from label to indices
    std::unordered_map<int64_t, std::vector<size_t>> label_to_indices_true;
    std::unordered_map<int64_t, std::vector<size_t>> label_to_indices_pred;

    for (size_t i = 0; i < n; ++i) {
        label_to_indices_true[labels_true[i]].push_back(i);
        label_to_indices_pred[labels_pred[i]].push_back(i);
    }

    // Contingency table
    std::unordered_map<int64_t, std::unordered_map<int64_t, size_t>> contingency;

    for (const auto& [label_true, indices_true] : label_to_indices_true) {
        for (const auto& [label_pred, indices_pred] : label_to_indices_pred) {
            size_t count = 0;
            // Compute intersection
            size_t i = 0, j = 0;
            size_t size_true = indices_true.size();
            size_t size_pred = indices_pred.size();
            while (i < size_true && j < size_pred) {
                if (indices_true[i] == indices_pred[j]) {
                    count++;
                    i++;
                    j++;
                }
                else if (indices_true[i] < indices_pred[j]) {
                    i++;
                }
                else {
                    j++;
                }
            }
            if (count > 0) {
                contingency[label_true][label_pred] = count;
            }
        }
    }

    // Compute sum of combinations
    auto comb2 = [](size_t x) -> double {
        return (x * (x - 1)) / 2.0;
    };

    double sum_comb_c = 0.0;
    for (const auto& [label_true, map_pred] : contingency) {
        for (const auto& [label_pred, count] : map_pred) {
            sum_comb_c += comb2(count);
        }
    }

    double sum_comb_a = 0.0;
    for (const auto& [label_true, indices_true] : label_to_indices_true) {
        sum_comb_a += comb2(indices_true.size());
    }

    double sum_comb_b = 0.0;
    for (const auto& [label_pred, indices_pred] : label_to_indices_pred) {
        sum_comb_b += comb2(indices_pred.size());
    }

    double comb_n = comb2(n);

    // Calculate ARI
    double expected_index = (sum_comb_a * sum_comb_b) / comb_n;
    double max_index = 0.5 * (sum_comb_a + sum_comb_b);
    double ari = (sum_comb_c - expected_index) / (max_index - expected_index);

    return ari;
}

double computeNMI(const std::vector<int64_t>& labels_true, const std::vector<int64_t>& labels_pred) {
    if (labels_true.size() != labels_pred.size()) {
        throw std::invalid_argument("Label vectors must have the same length.");
    }

    size_t n = labels_true.size();

    // Mapping from label to count
    std::unordered_map<int64_t, size_t> label_counts_true;
    std::unordered_map<int64_t, size_t> label_counts_pred;

    for (size_t i = 0; i < n; ++i) {
        label_counts_true[labels_true[i]]++;
        label_counts_pred[labels_pred[i]]++;
    }

    // Contingency table
    std::unordered_map<int64_t, std::unordered_map<int64_t, size_t>> contingency;

    for (size_t i = 0; i < n; ++i) {
        contingency[labels_true[i]][labels_pred[i]]++;
    }

    // Compute Mutual Information
    double mi = 0.0;
    for (const auto& [label_true, map_pred] : contingency) {
        for (const auto& [label_pred, count] : map_pred) {
            double p_ij = static_cast<double>(count) / n;
            double p_i = static_cast<double>(label_counts_true[label_true]) / n;
            double p_j = static_cast<double>(label_counts_pred[label_pred]) / n;
            mi += p_ij * std::log(p_ij / (p_i * p_j) + 1e-12); // Added epsilon to avoid log(0)
        }
    }

    // Compute Entropies
    double entropy_true = 0.0;
    for (const auto& [label_true, count_true] : label_counts_true) {
        double p_i = static_cast<double>(count_true) / n;
        entropy_true -= p_i * std::log(p_i + 1e-12);
    }

    double entropy_pred = 0.0;
    for (const auto& [label_pred, count_pred] : label_counts_pred) {
        double p_j = static_cast<double>(count_pred) / n;
        entropy_pred -= p_j * std::log(p_j + 1e-12);
    }

    // Compute NMI
    double nmi = 0.0;
    if (entropy_true + entropy_pred > 0.0) {
        nmi = 2.0 * mi / (entropy_true + entropy_pred);
    }

    return nmi;
}
