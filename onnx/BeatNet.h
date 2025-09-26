#ifndef BEATNETC_H
#define BEATNETC_H

#include <vector>
#include <string>
#include "onnxruntime_c_api.h"
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

using OrtGetApiBaseFn = const OrtApiBase* (*)();
using OrtCreateTensorWithDataAsOrtValueFn = OrtStatus* (*)
(
    const OrtMemoryInfo*, 
    void*, 
    size_t, 
    const int64_t*, 
    size_t, 
    ONNXTensorElementDataType, 
    OrtValue**
);
using OrtRunFn = OrtStatus* (*)
(
    OrtSession*,
    const OrtRunOptions*,
    const char* const*,
    const OrtValue* const*,
    size_t,
    const char* const*,
    size_t,
    OrtValue**
);
using OrtGetTensorMutableDataFn = OrtStatus* (*)(OrtValue*,void**);
using OrtReleaseValueFn = void (*)(OrtValue*);
using OrtGetTensorTypeAndShapeFn = OrtStatus* (*)(const OrtValue*, OrtTensorTypeAndShapeInfo**) noexcept;
using OrtGetDimensionsCountFn = OrtStatus* (*)(const OrtTensorTypeAndShapeInfo*, size_t*);
using OrtGetDimensionsFn = OrtStatus* (*)(const OrtTensorTypeAndShapeInfo*, int64_t*, size_t);
using OrtReleaseTensorTypeAndShapeInfoFn = void (*)(OrtTensorTypeAndShapeInfo*);
using OrtCreateEnvFn = OrtStatus* (ORT_API_CALL *)(OrtLoggingLevel, const char*, OrtEnv**) noexcept;
using OrtCreateSessionOptionsFn = OrtStatus* (ORT_API_CALL *)(OrtSessionOptions**);
using OrtSetIntraOpNumThreadsFn = OrtStatus* (ORT_API_CALL *)(OrtSessionOptions*, int);
using OrtCreateRunOptionsFn = OrtStatus* (ORT_API_CALL *)(OrtRunOptions**);
using OrtCreateCpuMemoryInfoFn = OrtStatus* (ORT_API_CALL *)(OrtAllocatorType, OrtMemType, OrtMemoryInfo**);
using OrtGetAllocatorWithDefaultOptionsFn = OrtStatus* (ORT_API_CALL *)(OrtAllocator**);
using OrtCreateSessionFn = OrtStatus* (ORT_API_CALL *)(const OrtEnv*, const ORTCHAR_T*, const OrtSessionOptions*, OrtSession**) noexcept;
using OrtSessionGetInputNameFn = OrtStatus* (ORT_API_CALL *)(const OrtSession*, size_t, OrtAllocator*, char**) noexcept;
using OrtSessionGetOutputNameFn = OrtStatus* (ORT_API_CALL *)(const OrtSession*, size_t, OrtAllocator*, char**) noexcept;
using OrtAllocatorFreeFn = OrtStatus* (ORT_API_CALL *)(OrtAllocator*, void*) noexcept;
using OrtReleaseSessionFn = void (ORT_API_CALL *)(OrtSession*);
using OrtReleaseSessionOptionsFn = void (ORT_API_CALL *)(OrtSessionOptions*);
using OrtReleaseMemoryInfoFn = void (ORT_API_CALL *)(OrtMemoryInfo*);
using OrtReleaseRunOptionsFn = void (ORT_API_CALL *)(OrtRunOptions*);
using OrtReleaseEnvFn = void (ORT_API_CALL *)(OrtEnv*);

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
    const OrtApi* ort = nullptr;
    OrtEnv* env;
    OrtSessionOptions* session_options;
    OrtSession* session;
    OrtAllocator* allocator;
    OrtMemoryInfo* memory_info;
    OrtRunOptions* run_options;
    const char* input_name;
    const char* output_name;

    // run-time linking
    void* onnxruntime_handle = nullptr;
    bool loadONNXRuntime(const std::string& libname);
    OrtCreateTensorWithDataAsOrtValueFn CreateTensorWithDataAsOrtValue = nullptr;
    OrtRunFn Run = nullptr;
    OrtGetTensorMutableDataFn GetTensorMutableData = nullptr;
    OrtReleaseValueFn ReleaseValue = nullptr;
    OrtGetTensorTypeAndShapeFn GetTensorTypeAndShape = nullptr;
    OrtGetDimensionsCountFn GetDimensionsCount = nullptr;
    OrtGetDimensionsFn GetDimensions = nullptr;
    OrtReleaseTensorTypeAndShapeInfoFn ReleaseTensorTypeAndShapeInfo = nullptr;
    OrtCreateEnvFn CreateEnv;
    OrtCreateSessionOptionsFn CreateSessionOptions;
    OrtSetIntraOpNumThreadsFn SetIntraOpNumThreads;
    OrtCreateRunOptionsFn CreateRunOptions;
    OrtCreateCpuMemoryInfoFn CreateCpuMemoryInfo;
    OrtGetAllocatorWithDefaultOptionsFn GetAllocatorWithDefaultOptions;
    OrtCreateSessionFn CreateSession;
    OrtSessionGetInputNameFn SessionGetInputName;
    OrtSessionGetOutputNameFn SessionGetOutputName;
    OrtAllocatorFreeFn AllocatorFree;
    OrtReleaseSessionFn ReleaseSession;
    OrtReleaseSessionOptionsFn ReleaseSessionOptions;
    OrtReleaseMemoryInfoFn ReleaseMemoryInfo;
    OrtReleaseRunOptionsFn ReleaseRunOptions;
    OrtReleaseEnvFn ReleaseEnv;

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
    void printOutputShape(OrtValue* output_tensors);

};

#endif