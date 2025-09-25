#include "BeatNet.h"
#include <filesystem>
#include <iostream>

BeatNet::BeatNet(
    const std::string& modelPath,
    const char* ortenvname,
    OrtLoggingLevel ortlogginglevel,
    int intraopnumthreads
): 
    ort(OrtGetApiBase()->GetApi(ORT_API_VERSION)),
    env(nullptr), session(nullptr), session_options(nullptr),
    memory_info(nullptr), allocator(nullptr), run_options(nullptr),
    input_name(nullptr), output_name(nullptr),
    signal_processor(FRAME_LENGTH, HOP_SIZE),
    fft_processor(FRAME_LENGTH, FFT_SIZE, FRAME_SIZE_POW2),
    filterbank_processor(BANKS_PER_OCTAVE, FFT_SIZE, SR_BEATNET, 30.0f, 11025.0f, true, true),
    SR(0),bufferSize(0)
{
    if (!std::filesystem::exists(modelPath)) {
        throw std::runtime_error("Model path does not exist: " + modelPath);
    }

    ort->CreateEnv(ORT_LOGGING_LEVEL_WARNING, "BeatNet", &env);
    ort->CreateSessionOptions(&session_options);
    ort->SetIntraOpNumThreads(session_options, 1);
    ort->CreateRunOptions(&run_options);
    ort->CreateCpuMemoryInfo(OrtArenaAllocator, OrtMemTypeDefault, &memory_info);
    ort->GetAllocatorWithDefaultOptions(&allocator);

#ifdef _WIN32
    std::wstring wModelPath(modelPath.begin(), modelPath.end());
    ort->CreateSession(env, wModelPath.c_str(), session_options, &session);
#else
    ort->CreateSession(env, modelPath.c_str(), session_options, &session);
#endif

    ort->SessionGetInputName(session, 0, allocator, const_cast<char**>(&this->input_name));
    ort->SessionGetOutputName(session, 0, allocator, const_cast<char**>(&this->output_name));

    preprocessed_input.resize(FBANK_SIZE);
    log_fb.resize(FBANK_SIZE / 2);
    diff.resize(FBANK_SIZE / 2);
    filters.resize(FBANK_SIZE / 2);
    spectrum.resize(FFT_SIZE);
    input_shape = {1, 1, FBANK_SIZE};
}

BeatNet::~BeatNet()
{
    if (input_name) ort->AllocatorFree(allocator, const_cast<char*>(input_name));
    if (output_name) ort->AllocatorFree(allocator, const_cast<char*>(output_name));
    if (session) ort->ReleaseSession(session);
    if (session_options) ort->ReleaseSessionOptions(session_options);
    if (memory_info) ort->ReleaseMemoryInfo(memory_info);
    if (run_options) ort->ReleaseRunOptions(run_options);
    if (env) ort->ReleaseEnv(env);
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

    ort->CreateTensorWithDataAsOrtValue(
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

    ort->Run(session, run_options,
             input_names, &input_tensor, 1,
             output_names, 1,
             &output_tensor);

    float* output_data = nullptr;
    ort->GetTensorMutableData(output_tensor, (void**)&output_data);

    output.resize(3);  // Assuming [1, 3, 1] shape
    for (int i = 0; i < 3; ++i) {
        output[i] = output_data[i];
    }

    printOutputShape(output_tensor);

    ort->ReleaseValue(input_tensor);
    ort->ReleaseValue(output_tensor);
}

void BeatNet::printOutputShape(OrtValue* output_tensor) {
    OrtTensorTypeAndShapeInfo* shape_info;
    ort->GetTensorTypeAndShape(output_tensor, &shape_info);

    size_t dim_count;
    ort->GetDimensionsCount(shape_info, &dim_count);
    std::vector<int64_t> shape(dim_count);
    ort->GetDimensions(shape_info, shape.data(), dim_count);

    std::cout << "Output shape: [";
    for (size_t i = 0; i < shape.size(); ++i) {
        std::cout << shape[i];
        if (i < shape.size() - 1) std::cout << ", ";
    }
    std::cout << "]" << std::endl;

    ort->ReleaseTensorTypeAndShapeInfo(shape_info);
}