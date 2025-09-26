#include "BeatNet.h"
#include <filesystem>
#include <iostream>

bool BeatNet::loadONNXRuntime(const std::string& dynamicLibName) {
    
    std::string libname;
    #if defined(_WIN32)
        libname = PluginUtils::makePlatformLibName("", dynamicLibName, ".dll");
    #elif defined(__APPLE__)
        libname = PluginUtils::makePlatformLibName("lib", dynamicLibName, ".dylib");
    #else
        libname = PluginUtils::makePlatformLibName("lib", dynamicLibName, ".so");
    #endif

    onnxruntime_handle = PluginUtils::loadDynamicLibrary(libname);
    if (!onnxruntime_handle) {
        std::cerr << "Failed to load ONNX Runtime library: " << libname << std::endl;
        return false;
    }

    auto getApiBase = reinterpret_cast<OrtGetApiBaseFn>(
        PluginUtils::getSymbol(onnxruntime_handle, "OrtGetApiBase")
    );
    if (!getApiBase) {
        std::cerr << "Failed to resolve OrtGetApiBase." << std::endl;
        return false;
    }
    
    ort = getApiBase()->GetApi(ORT_API_VERSION);
    if (!ort) {
        std::cerr << "Failed to get ONNX API." << std::endl;
        return false;
    }

    CreateTensorWithDataAsOrtValue = ort->CreateTensorWithDataAsOrtValue;
    Run = ort->Run;
    GetTensorMutableData = ort->GetTensorMutableData;
    ReleaseValue = ort->ReleaseValue;
    GetTensorTypeAndShape = ort->GetTensorTypeAndShape;
    GetDimensionsCount = ort->GetDimensionsCount;
    GetDimensions = ort->GetDimensions;
    ReleaseTensorTypeAndShapeInfo = ort->ReleaseTensorTypeAndShapeInfo;

    CreateEnv = ort->CreateEnv;
    CreateSessionOptions = ort->CreateSessionOptions;
    SetIntraOpNumThreads = ort->SetIntraOpNumThreads;
    CreateRunOptions = ort->CreateRunOptions;
    CreateCpuMemoryInfo = ort->CreateCpuMemoryInfo;
    GetAllocatorWithDefaultOptions = ort->GetAllocatorWithDefaultOptions;
    CreateSession = ort->CreateSession;
    SessionGetInputName = ort->SessionGetInputName;
    SessionGetOutputName = ort->SessionGetOutputName;
    AllocatorFree = ort->AllocatorFree;
    ReleaseSession = ort->ReleaseSession;
    ReleaseSessionOptions = ort->ReleaseSessionOptions;
    ReleaseMemoryInfo = ort->ReleaseMemoryInfo;
    ReleaseRunOptions = ort->ReleaseRunOptions;
    ReleaseEnv = ort->ReleaseEnv;

    return true;
}

BeatNet::BeatNet(
    const std::string& modelPath,
    const char* ortenvname,
    OrtLoggingLevel ortlogginglevel,
    int intraopnumthreads
): 
    env(nullptr), session(nullptr), session_options(nullptr),
    memory_info(nullptr), allocator(nullptr), run_options(nullptr),
    input_name(nullptr), output_name(nullptr),
    signal_processor(FRAME_LENGTH, HOP_SIZE),
    fft_processor(FRAME_LENGTH, FFT_SIZE, FRAME_SIZE_POW2),
    filterbank_processor(BANKS_PER_OCTAVE, FFT_SIZE, SR_BEATNET, 30.0f, 11025.0f, true, true),
    SR(0),bufferSize(0)
{

    if (!loadONNXRuntime("onnxruntime")) {
        throw std::runtime_error("Failed to load ONNX Runtime dynamically.");
    }

    if (!std::filesystem::exists(modelPath)) {
        throw std::runtime_error("Model path does not exist: " + modelPath);
    }

    CreateEnv(ORT_LOGGING_LEVEL_WARNING, "BeatNet", &env);
    CreateSessionOptions(&session_options);
    SetIntraOpNumThreads(session_options, 1);
    CreateRunOptions(&run_options);
    CreateCpuMemoryInfo(OrtArenaAllocator, OrtMemTypeDefault, &memory_info);
    GetAllocatorWithDefaultOptions(&allocator);

#ifdef _WIN32
    std::wstring wModelPath(modelPath.begin(), modelPath.end());
    CreateSession(env, wModelPath.c_str(), session_options, &session);
#else
    CreateSession(env, modelPath.c_str(), session_options, &session);
#endif

    SessionGetInputName(session, 0, allocator, const_cast<char**>(&this->input_name));
    SessionGetOutputName(session, 0, allocator, const_cast<char**>(&this->output_name));

    preprocessed_input.resize(FBANK_SIZE);
    log_fb.resize(FBANK_SIZE / 2);
    diff.resize(FBANK_SIZE / 2);
    filters.resize(FBANK_SIZE / 2);
    spectrum.resize(FFT_SIZE);
    input_shape = {1, 1, FBANK_SIZE};
}

BeatNet::~BeatNet()
{
    if (input_name) AllocatorFree(allocator, const_cast<char*>(input_name));
    if (output_name) AllocatorFree(allocator, const_cast<char*>(output_name));
    if (session) ReleaseSession(session);
    if (session_options) ReleaseSessionOptions(session_options);
    if (memory_info) ReleaseMemoryInfo(memory_info);
    if (run_options) ReleaseRunOptions(run_options);
    if (env) ReleaseEnv(env);

    if (onnxruntime_handle) {
        PluginUtils::unloadDynamicLibrary(onnxruntime_handle);
        onnxruntime_handle = nullptr;
    }

}


void BeatNet::setup(double sampleRate, int samplesPerBlock) {
    SR = sampleRate;
    bufferSize = samplesPerBlock;
    resampler.setup(SR, SR_BEATNET, bufferSize);
}

bool BeatNet::preprocess(const std::vector<float>& raw_input, std::vector<float>& preprocessed_input) {
        
    std::vector<float> resampled = resampler.resample(raw_input);
    std::vector<float> frame;
    bool valid_frame = signal_processor.process(resampled,frame);
    if (!valid_frame) {
        // std::cout<<"invalid frame and will be invalid for the first ~"<<FRAME_LENGTH/resampled.size()-1<<" frames"<<std::endl;
        return false;
    }
        
    spectrum = fft_processor.compute_fft(frame);
    filters = filterbank_processor.apply(spectrum);
    log_fb = log_compress(filters);
    diff = spectral_diff(log_fb, prev_log_fb);
    hstack(log_fb, diff, preprocessed_input);
    return true;
}

bool BeatNet::process(const std::vector<float>& raw_input, std::vector<float>& output) {
    if (!preprocess(raw_input, preprocessed_input)) {
        return false;
    }

    inference(output);
    return true;
}

void BeatNet::inference(std::vector<float>& output) {
    OrtValue* input_tensor = nullptr;

    CreateTensorWithDataAsOrtValue(
        memory_info,
        preprocessed_input.data(),
        preprocessed_input.size() * sizeof(float),
        input_shape.data(),
        input_shape.size(),
        ONNX_TENSOR_ELEMENT_DATA_TYPE_FLOAT,
        &input_tensor
    );

    OrtValue* output_tensor = nullptr;
    const char* input_names[] = { input_name };
    const char* output_names[] = { output_name };

    Run(session, run_options,
             input_names, &input_tensor, 1,
             output_names, 1,
             &output_tensor);

    float* output_data = nullptr;
    GetTensorMutableData(output_tensor, (void**)&output_data);

    output.resize(3);  // Assuming [1, 3, 1] shape
    for (int i = 0; i < 3; ++i) {
        output[i] = output_data[i];
    }

    printOutputShape(output_tensor);

    ReleaseValue(input_tensor);
    ReleaseValue(output_tensor);
}

void BeatNet::printOutputShape(OrtValue* output_tensor) {
    OrtTensorTypeAndShapeInfo* shape_info;
    GetTensorTypeAndShape(output_tensor, &shape_info);

    size_t dim_count;
    GetDimensionsCount(shape_info, &dim_count);
    std::vector<int64_t> shape(dim_count);
    GetDimensions(shape_info, shape.data(), dim_count);

    std::cout << "Output shape: [";
    for (size_t i = 0; i < shape.size(); ++i) {
        std::cout << shape[i];
        if (i < shape.size() - 1) std::cout << ", ";
    }
    std::cout << "]" << std::endl;

    ReleaseTensorTypeAndShapeInfo(shape_info);
}