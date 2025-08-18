#include "frameprocessor.h"
#include <algorithm>
#include <stdexcept>

#define MS_FR_PAPER 0.093
#define MS_HOP_PAPER 0.046 
#define MS_FR_GITHUB 0.064
#define MS_HOP_GITHUB 0.020

FramedSignalProcessor::FramedSignalProcessor(int sample_rate)
    : sample_rate(sample_rate),
      frame_size(static_cast<int>(sample_rate * static_cast<float>(MS_FR_GITHUB))),
      hop_size(static_cast<int>(sample_rate * static_cast<float>(MS_HOP_GITHUB))),
      ring_size(frame_size*1.1),            // make it a litle bit longer
      write_pos(0),
      ring_buffer(ring_size, 0.0f),
      total_samples_written(0){}

void FramedSignalProcessor::reset() {
    std::fill(ring_buffer.begin(), ring_buffer.end(), 0.0f);
    write_pos = 0;
    total_samples_written = 0;
}

bool FramedSignalProcessor::process(const std::vector<float>& input, std::vector<float>& frame_out) {

    for (float sample : input) {
        ring_buffer[write_pos] = sample;
        write_pos = (write_pos + 1) % ring_size;
        total_samples_written++;
    }

    if (total_samples_written>=frame_size)
    {
        frame_out.resize(frame_size);
        for (int i = 0; i < frame_size; ++i) {
            int index = (write_pos + ring_size - frame_size + i) % ring_size;
            frame_out[i] = ring_buffer[index];
        }
        return true;
    }
    else return false;
}