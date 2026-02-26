#ifndef FRAMEPROCESSOR_H
#define FRAMEPROCESSOR_H

#include <vector>

class FramedSignalProcessor {
public:

    FramedSignalProcessor(const std::vector<float>& inputSignal, int nFrames, int frameSize, int hopSize);
    ~FramedSignalProcessor();

    std::vector<float> operator[](int i);
    std::vector<float> getOriginalSignal();
    int get_nFrames();

    //FramedSignalProcessor(int frameSize, int hopSize);

    //bool process(const std::vector<float>& input, std::vector<float>& frame_out);

    //void reset();

private:
    //int sample_rate;

    //int frame_size;
    //int hop_size;
    //int ring_size;
    //int write_pos;
    //size_t total_samples_written;

    //std::vector<float> ring_buffer;

    
    std::vector<float> original_signal;
    int nFrames;
    int frameSize;
    int hopSize;

    std::vector<float> padded_signal;
    std::vector<std::vector<float>> frames;


};

#endif
