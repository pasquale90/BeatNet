#include "filterbankprocessor.h"
#include <algorithm>

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
    // centerfrequencies (219)
    int num_filters = static_cast<int>(std::floor(num_octaves * bands_per_octave)); // (219)

    /* 
        # get the range
    left = np.floor(np.log2(float(fmin) / fref) * bands_per_octave)
    right = np.ceil(np.log2(float(fmax) / fref) * bands_per_octave)
    # generate frequencies
    frequencies = fref * 2. ** (np.arange(left, right) /
                                float(bands_per_octave))
    # filter frequencies
    # needed, because range might be bigger because of the use of floor/ceil
    frequencies = frequencies[np.searchsorted(frequencies, fmin):]
    frequencies = frequencies[:np.searchsorted(frequencies, fmax, 'right')]
    
    */
    const float fref = 440.0; // 440Hz reference value in madmom python code
    float left = std::floor(std::log2(fmin / fref) * bands_per_octave);
    float right = std::ceil(std::log2(fmax / fref) * bands_per_octave);

    std::vector<float> centers(num_filters);
    float val = left + 1.0f; // left + 1 to skip the first value which is < fmin
    std::generate(centers.begin(), centers.end(),
                    [&val, fref, this]() 
                    {
                        return fref * std::pow(2.0f, val++ / (float) bands_per_octave); 
                    });


    //std::vector<float> frequencies1(num_filters);
    //for (int i = 0, istart = left +1; i < num_filters; ++i, ++istart)
    //{
    //    frequencies1[i] = fref * std::pow(2.0f, (float)istart / (float)bands_per_octave);
    //}

    std::vector<int> bins = centersHzToBins(centers);



    //frequencies to bins = num filters + 2 (138)
    //TODO

    // nfilters = 138 - 2 = 136
    //std::vector<float> centers(num_filters + 2);
    //std::vector<float> centers(num_filters);
    //for (int i = 1; i < num_filters + 1; ++i) {
    //    centers[i-1] = fmin * std::pow(2.0, (float)i / (float)bands_per_octave);
    //}

    //TODO
    for (int i = 1; i < bins.size() - 1; ++i) {


        std::vector<float> filt(fft_size, 0.0f); // std::vector<float> filt(fft_size / 2 + 1, 0.0);

        int l = bins[i - 1];  // float l = hzToBin(centers[i - 1]);
        int c = bins[i];      // float c = hzToBin(centers[i]);
        int r = bins[i + 1];  // float r = hzToBin(centers[i + 1]);

        int start = l;
        int center = c - l; // relative to start
        int stop = r - l;  // relative to start

        /*
        data = np.zeros(stop)
        # rising edge (without the center)
        data[:center] = np.linspace(0, 1, center, endpoint=False)
        # falling edge (including the center, but without the last bin)
        data[center:] = np.linspace(1, 0, stop - center, endpoint=False)
        
        */
        int n = stop;
        std::vector<float> data(n, 0.0f);

        float dx = 1.0f / center;

        // rising edge(without the center)
        float x0 = 0.0f;
        for (int i = 0; i < center; ++i)
        {
            data[i] = x0 + (i * dx);            
        }

        // falling edge (including the center, but without the last bin)
        x0 = 1.0f;
        for (int i = center; i < stop; ++i)
        {
            data[i] = x0 - ( (i - center) * dx);
        }

        std::copy(data.begin(), data.end(), filt.begin() + start);

        //for (int j = (int)std::ceil(l); j < (int)std::ceil(c) && j < filt.size(); ++j)
        //    filt[j] = (j - l) / (c - l);

        //for (int j = (int)std::ceil(c); j < (int)std::ceil(r) && j < filt.size(); ++j)
        //    filt[j] = (r - j) / (r - c);

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

//float FilterBankProcessor::hzToBin(float f) const {
//    return (f / (float)sample_rate) * fft_size;
//}

std::vector<int> FilterBankProcessor::centersHzToBins(const std::vector<float>& centers) const {

    std::vector<int> bins(centers.size());
    for (int i= 0; i < bins.size(); ++i)
    {
        const float value =  std::round( centers[i] / ((float) sample_rate / 2.0f)* fft_size);
        bins[i] = static_cast<int>(value);
    }

    // keep values unique 
    auto newend = std::unique(bins.begin(), bins.end());
    bins.erase(newend, bins.end());

    // remove values higher than fft_size
    const int size_max = fft_size - 1;
    newend = std::remove_if(bins.begin(), bins.end(), [&size_max](int x) {return x > size_max;});
    bins.erase(newend, bins.end());

    // add the size_max value at the end of the array
    bins.push_back(size_max);

    return bins;    
}