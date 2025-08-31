#include "fftprocessor.h"
#include <cmath>
#include <cassert>

#define M_PI 3.14159

#ifdef ENABLE_FFTW3
FFTProcessor::FFTProcessor(int frameSize, int fftSize, int max_frameSize_pow2): 
    frame_size(frameSize), 
    fft_size(fftSize),
    hann_window(frameSize),
    magnitudes(fftSize),
    frame_size_padded(max_frameSize_pow2)
{
    // Allocate FFT buffers
    fft_input = (float*)fftwf_malloc(sizeof(float) * frame_size_padded);
    fft_output = (fftwf_complex*)fftwf_malloc(sizeof(fftwf_complex) * (frame_size_padded / 2 + 1));

    // Create FFT plan
    fft_plan = fftwf_plan_dft_r2c_1d(frame_size_padded, fft_input, fft_output, FFTW_MEASURE);

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

    // zero padd rest of the input samples
    for (int i = frame_size; i < frame_size_padded; ++i)
        fft_input[i] = 0.0f;

    fftwf_execute(fft_plan);

    // take only the first ones...
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

#endif
#ifdef ENABLE_KISSFFT

FFTProcessor::FFTProcessor(int frameSize, int fftSize, int max_frameSize_pow2):
    frame_size(frameSize),
    fft_size(fftSize),
    hann_window(frameSize),
    magnitudes(fftSize),
    frame_size_padded(max_frameSize_pow2)
{
    fft_cfg = kiss_fftr_alloc(frame_size_padded, 0, nullptr, nullptr);

    fft_input = new kiss_fft_scalar[frame_size_padded];
    fft_output = new kiss_fft_cpx[frame_size_padded / 2 + 1];

    initialize_hann_window();
}

FFTProcessor::~FFTProcessor()
{
    if (fft_cfg)
        free(fft_cfg);

    delete[] fft_input;
    delete[] fft_output;
}

void FFTProcessor::initialize_hann_window()
{
    for (int i = 0; i < frame_size; ++i)
        hann_window[i] = 0.5f * (1.0f - std::cos(2.0f * M_PI * i / (frame_size - 1)));
}

std::vector<float> FFTProcessor::compute_fft(const std::vector<float>& input_frame) {

    assert(input_frame.size() == frame_size);

    // apply window to the input signal
    for (int i = 0; i < frame_size; ++i) 
        fft_input[i] = input_frame[i] * hann_window[i];

    // and then zero padd the rest of the fft input signal to fill the <pow of 2> sized input.
    for (int i = frame_size; i < frame_size_padded; ++i)
        fft_input[i] = 0.0f;

    kiss_fftr(fft_cfg, fft_input, fft_output);

    for (int i = 0; i < fft_size; ++i) {
        float real = fft_output[i].r;
        float imag = fft_output[i].i;
        magnitudes[i] = std::sqrt(real * real + imag * imag);
    }

    return magnitudes;
}

#endif