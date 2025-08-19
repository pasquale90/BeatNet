#include "fftprocessor.h"
#include <cmath>
#include <cassert>

#define M_PI 3.14159

FFTProcessor::FFTProcessor(int frameSize, int fftSize): 
    frame_size(frameSize), 
    fft_size(fftSize),
    hann_window(frameSize),
    magnitudes(fftSize)
{
    // Allocate FFT buffers
    fft_input = (float*)fftwf_malloc(sizeof(float) * fft_size);
    fft_output = (fftwf_complex*)fftwf_malloc(sizeof(fftwf_complex) * (fft_size / 2 + 1));

    // Create FFT plan
    fft_plan = fftwf_plan_dft_r2c_1d(fft_size, fft_input, fft_output, FFTW_MEASURE);

    // Generate Hann window
    initialize_hann_window();
}

FFTProcessor::~FFTProcessor()
{
    fftwf_destroy_plan(fft_plan);
    fftwf_free(fft_input);
    fftwf_free(fft_output);
}

void FFTProcessor::initialize_hann_window()
{
    for (int i = 0; i < frame_size; ++i)
        hann_window[i] = 0.5f * (1.0f - std::cos(2.0f * M_PI * i / (frame_size - 1)));
}

std::vector<float> FFTProcessor::compute_fft(const std::vector<float>& input_frame) {
    
    assert(input_frame.size() == frame_size);
    
    for (int i = 0; i < frame_size; ++i)
        fft_input[i] = input_frame[i] * hann_window[i]; // copy to fft input buffer

    fftwf_execute(fft_plan);

    for (int i = 0; i < fft_size; ++i) {
        float real = fft_output[i][0];
        float imag = fft_output[i][1];
        magnitudes[i] = std::sqrt(real * real + imag * imag);
    }

    // std::cout << "FFT Magnitudes (first 10 ): ";
    // for (int i = 0; i < 10; ++i) {
    //     std::cout << magnitudes[i] << " ";
    // }
    // std::cout << std::endl;

    return magnitudes;
}