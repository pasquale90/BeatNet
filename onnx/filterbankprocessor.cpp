#include "filterbankprocessor.h"

FilterBankProcessor::FilterBankProcessor(
    int bands_per_octave, 
    int fft_size, 
    int sample_rate,
    float fmin, 
    float fmax,
    bool norm_filters, 
    bool unique_filters): 
        bands_per_octave(bands_per_octave),
        fft_size(fft_size),
        sample_rate(sample_rate),
        fmin(fmin),
        fmax(fmax),
        norm_filters(norm_filters),
        unique_filters(unique_filters)
{
    buildFilters();
}

void FilterBankProcessor::buildFilters() {
    filters.clear();
    float num_octaves = std::log2(fmax / fmin);
    int num_filters = static_cast<int>(std::floor(num_octaves * bands_per_octave));
    std::vector<float> centers(num_filters + 2);

    for (int i = 0; i < centers.size(); ++i) {
        centers[i] = fmin * std::pow(2.0, (float)i / (float)bands_per_octave);
    }

    for (int i = 1; i < centers.size() - 1; ++i) {
        std::vector<float> filt(fft_size / 2 + 1, 0.0);
        float l = hzToBin(centers[i - 1]);
        float c = hzToBin(centers[i]);
        float r = hzToBin(centers[i + 1]);

        for (int j = (int)std::ceil(l); j < (int)std::ceil(c) && j < filt.size(); ++j)
            filt[j] = (j - l) / (c - l);

        for (int j = (int)std::ceil(c); j < (int)std::ceil(r) && j < filt.size(); ++j)
            filt[j] = (r - j) / (r - c);

        if (norm_filters) {
            float sum = std::accumulate(filt.begin(), filt.end(), 0.0);
            if (sum > 0)
                for (auto &v : filt) v /= sum;
        }

        filters.push_back(std::move(filt));
    }
}

std::vector<float> FilterBankProcessor::apply(const std::vector<float> &spectrum) const {
    std::vector<float> out(filters.size(), 0.0);
    for (size_t i = 0; i < filters.size(); ++i) {
        for (size_t j = 0; j < spectrum.size() && j < filters[i].size(); ++j) {
            out[i] += spectrum[j] * filters[i][j];
        }
    }
    return out;
}

int FilterBankProcessor::numBands() const 
{ 
    return (int)filters.size();
}

float FilterBankProcessor::hzToBin(float f) const {
    return (f / (float)sample_rate) * fft_size;
}