#ifndef FFT_PROCESSOR_H
#define FFT_PROCESSOR_H

#include <vector>
#include <string>
#include <iostream>
#ifdef ENABLE_FFTW3
#include <fftw3.h>
using fftwf_malloc_t = void* (*)(size_t);
using fftwf_free_t = void (*)(void*);
using fftwf_plan_dft_r2c_1d_t = fftwf_plan (*)(int, float*, fftwf_complex*, unsigned);
using fftwf_execute_t = void (*)(const fftwf_plan);
using fftwf_destroy_plan_t = void (*)(fftwf_plan);
#endif 
#ifdef ENABLE_KISSFFT
#include <kiss_fft.h>
#include <kiss_fftr.h>
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

#ifdef ENABLE_FFTW3
    float* fft_input;
    fftwf_complex* fft_output;
    fftwf_plan fft_plan;

    bool loadLibfftw3();
    const std::string dynamiclibname = "fftw3f-3";
    void* fftw_handle = nullptr;
    fftwf_malloc_t fftwf_malloc_func = nullptr;
    fftwf_free_t fftwf_free_func = nullptr;
    fftwf_plan_dft_r2c_1d_t fftwf_plan_dft_r2c_1d_func = nullptr;
    fftwf_execute_t fftwf_execute_func = nullptr;
    fftwf_destroy_plan_t fftwf_destroy_plan_func = nullptr;

#endif
#ifdef ENABLE_KISSFFT
    kiss_fftr_cfg fft_cfg;
    kiss_fft_scalar* fft_input;
    kiss_fft_cpx* fft_output;
#endif

    void initialize_hann_window();
    std::vector<float> magnitudes;
};

#endif
