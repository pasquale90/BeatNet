#ifndef BEATNET_H
#define BEATNET_H

#include <vector>
#include <string>
#include "onnxruntime_cxx_api.h"
#include "samplerate.h"
#include "resampler.h"
#include "frameprocessor.h"
#include "fftprocessor.h"
#include "filterbankprocessor.h"
#include "logspecutils.h"

constexpr int SR_BEATNET {22050}; 
constexpr double MS_FR_PAPER {0.093};
constexpr double MS_HOP_PAPER {0.046}; 
constexpr double MS_FR_GITHUB {0.064};
constexpr double MS_HOP_GITHUB {0.020};
constexpr int FRAME_LENGTH {static_cast<int>(SR_BEATNET*MS_FR_GITHUB)}; // 1411
constexpr int HOP_SIZE {static_cast<int>(SR_BEATNET*MS_HOP_GITHUB)}; // 441
constexpr int FFT_SIZE { FRAME_LENGTH / 2 + 1}; // 706
constexpr int FRAME_SIZE_POW2 {2048}; // this is the minumum higher than FRAME_LENGTH (1411) that is a power-of-two value.
constexpr int FBANK_SIZE {272};
constexpr int BANKS_PER_OCTAVE {16}; // {24};;

class BeatNet{
public:
    BeatNet(
        const std::string& modelPath,
        const char* ortenvname = "BeatNet",
        OrtLoggingLevel ortlogginglevel=ORT_LOGGING_LEVEL_WARNING,
        int intraopnumthreads =1
    );
    ~BeatNet();

    void setup(double sampleRate, int samplesPerBlock);

    bool process(const std::vector<float>& raw_input, std::vector<float>& output);
    
private:
    float SR;
    int bufferSize;

    // ONNX Runtime
    Ort::Env env;
    Ort::SessionOptions session_options;
    Ort::Session session;
    Ort::AllocatorWithDefaultOptions allocator;
    Ort::MemoryInfo memory_info;
    Ort::RunOptions run_options;
    const char* input_name;
    const char* output_name;

    // Preprocessing
    Resampler resampler;
    FramedSignalProcessor signal_processor;
    FFTProcessor fft_processor;
    FilterBankProcessor filterbank_processor;
    std::vector<float> preprocessed_input;
    std::vector<int64_t> input_shape;
    std::vector<float> spectrum;
    std::vector<float> filters;
    std::vector<float> log_fb;
    std::vector<float> diff;
    std::vector<float> prev_log_fb;

    // helper functions - preprocess for feature extraction and inference for model utilization
    bool preprocess(const std::vector<float>& raw_input, std::vector<float>& preprocessed_input);
    void inference(std::vector<float>& output);
    void printOutputShape(std::vector<Ort::Value> &output_tensors);

};

#endif