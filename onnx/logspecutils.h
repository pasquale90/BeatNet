#ifndef LOGSPECUTILS_H
#define LOGSPECUTILS_H

#pragma once
#include <vector>

// Log compression: log10(mul * x + add)
std::vector<float> log_compress(const std::vector<float>& input, float mul = 1.0f, float add = 1.0f);

// Compute first-order spectral difference (current - previous)
std::vector<float> spectral_diff(const std::vector<float>& current,
                                 std::vector<float>& previous,
                                 bool positive_diffs = true);

// Horizontally stack two vectors
std::vector<float> hstack(const std::vector<float>& a, const std::vector<float>& b);

#endif