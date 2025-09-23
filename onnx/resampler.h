#pragma once

#include <vector>
#include <string>
#include "samplerate.h"
#include "dynamic_link.h"

// type aliases
using src_callback_t = long (*)(void* cb_data, float** data);
using src_callback_new_t  = SRC_STATE* (*)(src_callback_t, int, int, int*, void*);
using src_strerror_t      = const char* (*)(int);
using src_callback_read_t = long (*)(SRC_STATE*, double, long, float*);

class Resampler {
public:
    Resampler(double input_sr, double output_sr, long bufferSize);
    Resampler();
    
    ~Resampler();
    Resampler(const Resampler&) = delete;     // copy constructor
    Resampler& operator=(const Resampler&) = delete;     // copy assignment
    Resampler(Resampler&&) = delete; //move constructor
    Resampler& operator=(Resampler&&) = delete; //move assignment


    void setup(double input_sr, double output_sr, long bufferSize);

    std::vector<float> resample(const std::vector<float>& input);

private:
    struct CallbackData {
        const float* input;
        long total_frames;
        long position;
    };

    static long callback(void* cb_data, float** data);

    CallbackData cb_data;
    int error;
    double ratio;
    long buffer_size;
    long output_frame_count;

    std::vector<float> silence, output_buffer;

    bool loadLibsamplerate();
    void* samplerate_handle = nullptr;
    const std::string dynamiclibname = "samplerate";
    src_callback_new_t  callback_new = nullptr;
    src_strerror_t      strerror = nullptr;
    src_callback_read_t callback_read = nullptr;

};




// SRC_STATE
// src_callback_new
// src_strerror
// src_callback_read(state_, ratio, output_frame_count, output_buffer.data());