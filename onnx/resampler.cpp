#include "resampler.h"
#include <iostream>
#include <algorithm>

Resampler::Resampler(){}

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
    
    SRC_STATE* state_ = src_callback_new(callback, SRC_SINC_FASTEST, 1, &error, &cb_data);
    if (!state_) {
        std::cerr << "libsamplerate error: " << src_strerror(error) << std::endl;
    }
    long frames_written = src_callback_read(state_, ratio, output_frame_count, output_buffer.data());
    return output_buffer;
}
