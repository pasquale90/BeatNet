#ifndef FFT_PROCESSOR_H
#define FFT_PROCESSOR_H

#include <vector>
#include <fftw3.h>

class FFTProcessor {
public:
    FFTProcessor(int frame_size);
    ~FFTProcessor();

    std::vector<float> compute_fft(const std::vector<float>& input_frame);

private:
    int frame_size;
    int fft_size;

    float* fft_input;
    fftwf_complex* fft_output;
    fftwf_plan fft_plan;
    std::vector<float> hann_window;

    void initialize_hann_window();
    std::vector<float> magnitudes;
};

#endif
