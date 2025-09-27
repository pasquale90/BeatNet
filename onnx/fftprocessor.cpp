#include "fftprocessor.h"
#include "dynamic_link.h"
#include <cmath>
#include <cassert>

#define M_PI 3.14159

#ifdef ENABLE_FFTW3

bool FFTProcessor::loadLibfftw3()
{
    std::string libname;
    #if defined(_WIN32)
        libname = PluginUtils::makePlatformLibName("lib", dynamiclibname+"-3", ".dll");
    #elif defined(__APPLE__)
        libname = PluginUtils::makePlatformLibName("lib", dynamiclibname, ".dylib");
    #else
        libname = PluginUtils::makePlatformLibName("lib", dynamiclibname, ".so");
    #endif

    fftw_handle = PluginUtils::loadDynamicLibrary(libname);
    if (!fftw_handle) {
        std::cerr << "Failed to load "<< libname << std::endl;
        return false;
    }

    // Load symbols
    fftwf_malloc_func = reinterpret_cast<fftwf_malloc_t>(PluginUtils::getSymbol(fftw_handle, "fftwf_malloc"));
    fftwf_free_func = reinterpret_cast<fftwf_free_t>(PluginUtils::getSymbol(fftw_handle, "fftwf_free"));
    fftwf_plan_dft_r2c_1d_func = reinterpret_cast<fftwf_plan_dft_r2c_1d_t>(PluginUtils::getSymbol(fftw_handle, "fftwf_plan_dft_r2c_1d"));
    fftwf_execute_func = reinterpret_cast<fftwf_execute_t>(PluginUtils::getSymbol(fftw_handle, "fftwf_execute"));
    fftwf_destroy_plan_func = reinterpret_cast<fftwf_destroy_plan_t>(PluginUtils::getSymbol(fftw_handle, "fftwf_destroy_plan"));

    if (!fftwf_malloc_func || !fftwf_free_func || !fftwf_plan_dft_r2c_1d_func || !fftwf_execute_func || !fftwf_destroy_plan_func) {
        std::cerr << "Failed to load one or more FFTW symbols" << std::endl;
        return false;
    }
    return true;
}
FFTProcessor::FFTProcessor(int frameSize, int fftSize, int max_frameSize_pow2): 
    frame_size(frameSize), 
    fft_size(fftSize),
    hann_window(frameSize),
    magnitudes(fftSize),
    frame_size_padded(max_frameSize_pow2)
{
    bool libffw3Loaded = loadLibfftw3();
    if (!libffw3Loaded)
    {
        std::cerr << "Could not load fftw3 dll"<<std::endl;
    }

    // Allocate FFT buffers
    fft_input = (float*)fftwf_malloc_func(sizeof(float) * frame_size_padded);
    fft_output = (fftwf_complex*)fftwf_malloc_func(sizeof(fftwf_complex) * (frame_size_padded / 2 + 1));

    // Create FFT plan
    fft_plan = fftwf_plan_dft_r2c_1d_func(frame_size_padded, fft_input, fft_output, FFTW_MEASURE);

    // Generate Hann window
    initialize_hann_window();
}

FFTProcessor::~FFTProcessor()
{
    fftwf_destroy_plan_func(fft_plan);
    fftwf_free_func(fft_input);
    fftwf_free_func(fft_output);

    if (fftw_handle)
    {
        PluginUtils::unloadDynamicLibrary(fftw_handle);
    }

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

    fftwf_execute_func(fft_plan);

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