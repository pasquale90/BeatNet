#include "logspecutils.h"
#include <cmath>
#include <algorithm>

std::vector<float> log_compress(const std::vector<float>& input, float mul, float add) {
    std::vector<float> output(input.size());
    for (size_t i = 0; i < input.size(); ++i) {
        float val = mul * input[i] + add;
        output[i] = std::log10(std::max(val, 1e-6f));  // epsilon to avoid log(0)
    }
    return output;
}

std::vector<float> spectral_diff(const std::vector<float>& current,
                                 std::vector<float>& previous,
                                 bool positive_diffs) {
    if (previous.empty()) {// first frame has no diff
        previous = current;
        return std::vector<float>(current.size(), 0.0f);  
    }

    std::vector<float> diff(current.size());
    for (size_t i = 0; i < current.size(); ++i) {
        float delta = current[i] - previous[i];
        diff[i] = positive_diffs ? std::max(0.0f, delta) : delta;
    }

    previous = current;  // update previous to use it in the next call
    return diff;
}

std::vector<float> hstack(const std::vector<float>& a, const std::vector<float>& b) {
    std::vector<float> out;
    out.reserve(a.size() + b.size());
    out.insert(out.end(), a.begin(), a.end());
    out.insert(out.end(), b.begin(), b.end());
    return out;
}
