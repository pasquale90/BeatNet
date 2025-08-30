#ifndef FFT_PROCESSOR_H
#define FFT_PROCESSOR_H

#include <vector>
#ifdef USE_FFTW3
#include <fftw3.h>
#endif 
#ifdef USE_KISS_FFT
#include <kissfft/kiss_fft.h>
#include <kissfft/kiss_fftr.h>
#endif

class FFTProcessor {
public:
    FFTProcessor(int frameSize, int fftSize, int max_frameSize_pow2);
    ~FFTProcessor();

    std::vector<float> compute_fft(const std::vector<float>& input_frame);

private:
    int frame_size,frame_size_padded;
    int fft_size;

    std::vector<float> hann_window;

#ifdef USE_FFTW3
    float* fft_input;
    fftwf_complex* fft_output;
    fftwf_plan fft_plan;
#endif
#ifdef USE_KISS_FFT
    kiss_fftr_cfg fft_cfg;
    kiss_fft_scalar* fft_input;
    kiss_fft_cpx* fft_output;
#endif

    void initialize_hann_window();
    std::vector<float> magnitudes;
};

#endif
