#ifndef FILTERBANK_H
#define FILTERBANK_H

#include <vector>
#include <cmath>
#include <numeric>
#include <algorithm>

class FilterBankProcessor {

public:
    FilterBankProcessor(int bands_per_octave, int fft_size, int sample_rate,
                        float fmin = 30.0, float fmax = 17000.0,
                        bool norm_filters = true, bool unique_filters = true);

    void buildFilters();

    std::vector<float> apply(const std::vector<float> &spectrum) const;

    int numBands() const;

private:
    int bands_per_octave;
    int fft_size;
    int sample_rate;
    float fmin;
    float fmax;
    bool norm_filters;
    bool unique_filters;
    std::vector<std::vector<float>> filters;

    float hzToBin(float f) const;

};


#endif

