#pragma once

#include <vector>
#include "samplerate.h"

class Resampler {
public:
    Resampler(double input_sr, double output_sr, long bufferSize);
    Resampler();
    
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
};
