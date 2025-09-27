#include "resampler.h"
#include <iostream>
#include <algorithm>

bool Resampler::loadLibsamplerate(){
    std::string libname;
    #if defined(_WIN32)
        libname = PluginUtils::makePlatformLibName("", dynamiclibname, ".dll");
    #elif defined(__APPLE__)
        libname = PluginUtils::makePlatformLibName("lib", dynamiclibname, ".dylib");
    #else
        libname = PluginUtils::makePlatformLibName("lib", dynamiclibname, ".so");
    #endif

    samplerate_handle = PluginUtils::loadDynamicLibrary(libname);
    if (!samplerate_handle) {
        std::cerr << "Failed to load libsamplerate\n";
        return false;
    }

    callback_new = reinterpret_cast<src_callback_new_t>(PluginUtils::getSymbol(samplerate_handle, "src_callback_new" ));
    callback_read = reinterpret_cast<src_callback_read_t>(PluginUtils::getSymbol(samplerate_handle, "src_callback_read"));
    strerror = reinterpret_cast<src_strerror_t>(PluginUtils::getSymbol(samplerate_handle, "src_strerror"));
    if (!callback_new || !callback_read || !strerror) {
        std::cerr << "One or more symbols failed to load.\n";
        return false;
    }
    return true;

}
Resampler::Resampler(){
    bool libsamplerateLoaded = loadLibsamplerate();
    if (!libsamplerateLoaded)
    {
        std::cerr << "Could not load libsamplerate"<<std::endl;
    }
}

Resampler::Resampler(double input_sr, double output_sr, long bufferSize ): 
    ratio(output_sr / input_sr), 
    buffer_size(bufferSize),
    output_frame_count(static_cast<long>(static_cast<float>(buffer_size) * ratio + 0.5)),// round to nearest
    silence(output_frame_count, 0.0f),
    output_buffer(output_frame_count, 0.0f)
{
    error = 0;
    output_frame_count = static_cast<long>(static_cast<float>(buffer_size) * ratio + 0.5); // round to nearest
    cb_data.total_frames = static_cast<long>(buffer_size);

    bool libsamplerateLoaded = loadLibsamplerate();
    if (!libsamplerateLoaded)
    {
        std::cerr << "Could not load libsamplerate"<<std::endl;
    }

}

Resampler::~Resampler()
{
    if (samplerate_handle)
    {
        PluginUtils::unloadDynamicLibrary(samplerate_handle);
    }
}

void Resampler::setup(double input_sr, double output_sr, long bufferSize)
{
    if (bufferSize>0){
        ratio = output_sr / input_sr; 
        buffer_size = bufferSize;
        output_frame_count = static_cast<long>(static_cast<float>(buffer_size) * ratio + 0.5) ; // round to nearest
        silence.resize(output_frame_count, 0.0f);
        output_buffer.resize(output_frame_count, 0.0f);
        error = 0;
        output_frame_count = static_cast<long>(static_cast<float>(buffer_size) * ratio + 0.5); // round to nearest
        cb_data.total_frames = static_cast<long>(buffer_size);
    }
}

long Resampler::callback(void* data, float** out_data) {
    
    CallbackData* cb = static_cast<CallbackData*>(data);

    if (cb->position >= cb->total_frames)
        return 0;

    long remaining = cb->total_frames - cb->position;

    *out_data = const_cast<float*>(cb->input + cb->position);
    cb->position += remaining;

    return remaining;
}

std::vector<float> Resampler::resample(const std::vector<float>& input) {

    if (input.empty()) {
        return silence;
    }

    cb_data.input = input.data();
    cb_data.position = 0;
    
    SRC_STATE* state_ = callback_new(callback, SRC_SINC_FASTEST, 1, &error, &cb_data);
    if (!state_) {
        std::cerr << "libsamplerate error: " << strerror(error) << std::endl;
    }
    long frames_written = callback_read(state_, ratio, output_frame_count, output_buffer.data());
    return output_buffer;
}
