#ifndef FRAMEDSIGNAL_H
#define FRAMEDSIGNAL_H

#include <vector>

class FramedSignal {
public:

    FramedSignal(const std::vector<float>& inputSignal, int nFrames, int frameSize, int hopSize);
    ~FramedSignal();

    std::vector<float> operator[](int i);
    std::vector<float> getOriginalSignal();
    int get_nFrames();

private:
    std::vector<float> original_signal;
    int nFrames;
    int frameSize;
    int hopSize;

    std::vector<float> padded_signal;
    std::vector<std::vector<float>> frames;
};

#endif
