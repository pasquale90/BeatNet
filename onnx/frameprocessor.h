#ifndef FRAMEPROCESSOR_H
#define FRAMEPROCESSOR_H

#include <vector>

class FramedSignalProcessor {
public:
    FramedSignalProcessor(int sample_rate);
    void reset();

    bool process(const std::vector<float>& input, std::vector<float>& frame_out);

private:
    int sample_rate;
    int frame_size;
    int hop_size;
    int ring_size;
    int write_pos;
    size_t total_samples_written;

    std::vector<float> ring_buffer;
};

#endif
