#include "BeatNet.h"
#include <filesystem>

BeatNet::BeatNet(
    const std::string& modelPath,
    const char* ortenvname,
    OrtLoggingLevel ortlogginglevel,
    int intraopnumthreads
): 
    env(ortlogginglevel, ortenvname),
    session_options(),
    session(nullptr),
    memory_info(Ort::MemoryInfo::CreateCpu(OrtArenaAllocator, OrtMemTypeDefault)),
    run_options(nullptr),
    signal_processor(FRAME_LENGTH, HOP_SIZE),
    fft_processor(FRAME_LENGTH, FFT_SIZE, FRAME_SIZE_POW2),
    filterbank_processor(BANKS_PER_OCTAVE, FFT_SIZE, SR_BEATNET, 30.0f, 11025.0f, true, true)
{
    if (!std::filesystem::exists(modelPath)) {
        throw std::runtime_error("Model path does not exist: " + modelPath);
    }

    session_options.SetIntraOpNumThreads(intraopnumthreads);

    std::wstring wModelPath(modelPath.begin(), modelPath.end());
    session = Ort::Session(env, wModelPath.c_str(), session_options);

    Ort::AllocatedStringPtr input_name_ptr = session.GetInputNameAllocated(0, allocator);
    Ort::AllocatedStringPtr output_name_ptr = session.GetOutputNameAllocated(0, allocator);
    input_name = input_name_ptr.release();
    output_name = output_name_ptr.release();

    preprocessed_input.resize(FBANK_SIZE);
    log_fb.resize(FBANK_SIZE / 2);
    diff.resize(FBANK_SIZE / 2);
    filters.resize(FBANK_SIZE / 2);
    spectrum.resize(FFT_SIZE);
    input_shape = {1, 1, FBANK_SIZE};
}

BeatNet::~BeatNet(){}

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

void BeatNet::inference(std::vector<float>& output)
{
        Ort::Value input_tensor = Ort::Value::CreateTensor<float>(
        memory_info,
        preprocessed_input.data(),
        preprocessed_input.size(),
        input_shape.data(),
        input_shape.size()
    );

    auto output_tensors = session.Run(
        run_options,
        &input_name,
        &input_tensor,
        1,
        &output_name,
        1
    );

    float* output_data = output_tensors[0].GetTensorMutableData<float>();
    output = std::vector<float>(output_data, output_data + 3); // Output shape: [1, 3, 1]

    printOutputShape(output_tensors);

}

#include <iostream>
void BeatNet::printOutputShape(std::vector<Ort::Value> &output_tensors){
    /*
     * batch size 1
     * 3 classes (Beat / Downbeat/ None)
     * 1  
     */
    Ort::Value& output_tensor = output_tensors[0];

    // Get tensor shape info
    auto type_info = output_tensor.GetTensorTypeAndShapeInfo();
    std::vector<int64_t> output_shape = type_info.GetShape();

    // Print shape
    std::cout << "Output shape: [";
    for (size_t i = 0; i < output_shape.size(); ++i) {
        std::cout << output_shape[i];
        if (i < output_shape.size() - 1) std::cout << ", ";
    }
    std::cout << "]" << std::endl;
}

