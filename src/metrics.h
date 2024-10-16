// metrics.h
#ifndef METRICS_H
#define METRICS_H

#include <vector>
#include <unordered_map>
#include <cmath>
#include <stdexcept>
#include <algorithm>

/**
 * @brief Computes the Adjusted Rand Index (ARI) between two clusterings.
 *
 * @param labels_true Ground truth labels.
 * @param labels_pred Predicted labels.
 * @return ARI score.
 */
double computeARI(const std::vector<int64_t>& labels_true, const std::vector<int64_t>& labels_pred);

/**
 * @brief Computes the Normalized Mutual Information (NMI) between two clusterings.
 *
 * @param labels_true Ground truth labels.
 * @param labels_pred Predicted labels.
 * @return NMI score.
 */
double computeNMI(const std::vector<int64_t>& labels_true, const std::vector<int64_t>& labels_pred);

#endif // METRICS_H
