#include "onnxruntime_cxx_api.h"
#include <iostream>
#include <string>
#include <filesystem>
#include "samplerate.h"
#include <random>
#include "resampler.h"
#include "frameprocessor.h"
#include "fftprocessor.h"
#include "filterbankprocessor.h"
#include "logspecutils.h"

constexpr int SR {96000}; // user defined
constexpr int SR_BEATNET {22050}; 
constexpr int BUFFER_SIZE {256}; // user defined
constexpr double MS_FR_PAPER {0.093};
constexpr double MS_HOP_PAPER {0.046}; 
constexpr double MS_FR_GITHUB {0.064};
constexpr double MS_HOP_GITHUB {0.020};
constexpr int FRAME_LENGTH {static_cast<int>(SR_BEATNET*MS_FR_GITHUB)};
constexpr int HOP_SIZE {static_cast<int>(SR_BEATNET*MS_HOP_GITHUB)};
constexpr int FFT_SIZE { FRAME_LENGTH / 2 + 1};
constexpr int FBANK_SIZE {272};
constexpr int BANKS_PER_OCTAVE {16}; // {24};
static Resampler resampler(SR, SR_BEATNET, BUFFER_SIZE);
static FramedSignalProcessor signal_processor(FRAME_LENGTH,HOP_SIZE);
static FFTProcessor fft_processor(FRAME_LENGTH, FFT_SIZE);
static FilterBankProcessor filterbank_processor(BANKS_PER_OCTAVE, FFT_SIZE, SR_BEATNET, 30.0f, 11025.0f, true, true);
        
std::string modelPath("beatnet_bda.onnx");

bool preprocess(std::vector<float> &raw_input, std::vector<float> &preprocessed_input){ 
    // apply preprocessing steps
    std::cout<<"Preprocessing"<<std::endl;

    // resample buffer to 22050 Hz
    std::vector<float> resampled = resampler.resample(raw_input);       
    std::cout<<"raw_input size equals to "<<raw_input.size()<<std::endl;
    std::cout<<"resampled size equals to "<<resampled.size()<<std::endl;

    // Apply framing
    std::vector<float> frame;
    bool valid_frame = signal_processor.process(resampled,frame);
    if (!valid_frame)
    {
        std::cout<<"invalid frame and will be invalid for the first ~"<<FRAME_LENGTH/resampled.size()-1<<" frames"<<std::endl;
        return false;
    }

    std::cout<<"valid frame - > processed_frames size = "<<frame.size()<<std::endl;
    
    // compute fft
    std::vector<float> spectrum = fft_processor.compute_fft(frame);
    std::cout<<"spectrum computed with spectrum size = "<<spectrum.size()<<std::endl;

    //filterbanks
    std::vector<float> filters = filterbank_processor.apply(spectrum);
    std::cout<<"filters computed with filters size = "<<filters.size()<<std::endl;

    // log spectrogram utils
    std::vector<float> log_fb = log_compress(filters);
    std::cout<<"log_compress size = "<<log_fb.size()<<std::endl;    
    static std::vector<float> prev_log_fb;
    std::vector<float> diff = spectral_diff(log_fb, prev_log_fb);
    std::cout<<"diff size = "<<diff.size()<<std::endl;
    std::vector<float> final_input = hstack(log_fb, diff);
    std::cout<<"final_input size = "<<final_input.size()<<std::endl;

    // set preprocessed_input - finally!
    std::copy(final_input.begin(), final_input.end(), preprocessed_input.begin());
}

float randomFloatGenerator() {
    return static_cast<float>(rand()) / RAND_MAX;
}

void printOutputShape(std::vector<Ort::Value> &output_tensors){
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

int main()
{
    std::cout << "Starting inference..." << std::endl;

    Ort::Env env(ORT_LOGGING_LEVEL_WARNING, "BeatNet"); //ORT_LOGGING_LEVEL_VERBOSE
    Ort::SessionOptions session_options;
    session_options.SetIntraOpNumThreads(1);

    std::cout << "Loading model: "<< modelPath << std::endl;
    if (!std::filesystem::exists(modelPath)) {
        std::cerr << "Error: model path "<<modelPath<<" does not exist"<<std::endl;
        return 1;
    }

    std::wstring wModelPath(modelPath.begin(), modelPath.end()); 
    Ort::Session session(env, wModelPath.c_str(),session_options); // #define ORTCHAR_T wchar_t
    Ort::AllocatorWithDefaultOptions allocator;

    std::cout << "Fetching input/output names..." << std::endl; // check https://netron.app/
    Ort::AllocatedStringPtr input_name_ptr = session.GetInputNameAllocated(0,allocator);
    Ort::AllocatedStringPtr output_name_ptr = session.GetOutputNameAllocated(0,allocator);
    const char* input_names[] = {input_name_ptr.get()};
    const char* output_names[] = {output_name_ptr.get()};
    std::cout << "Input name: " << input_names[0] << std::endl;
    std::cout << "Output name: " << output_names[0] << std::endl;

    std::cout << "Extracting features..." << std::endl;
    // input raw data
    int numBuffers = 30;
    std::vector<float> raw_input(BUFFER_SIZE);
    std::vector<int64_t> input_shape = {1,1,FBANK_SIZE}; 
    std::vector<float> preprocessed_input(input_shape[0] * input_shape[1] * input_shape[2]);
    Ort::MemoryInfo memory_info = Ort::MemoryInfo::CreateCpu(OrtArenaAllocator, OrtMemTypeDefault);
    Ort::RunOptions run_options{nullptr};
    
    for (int i=0; i<numBuffers; ++i){
        
        std::cout << "\nIteration: " << i << std::endl;
        std::cout<<"raw_input size "<<raw_input.size()<<std::endl;
        std::cout<<"preprocessed_input size "<<preprocessed_input.size()<<std::endl;
        
        std::generate(raw_input.begin(), raw_input.end(), randomFloatGenerator);
        std::generate(preprocessed_input.begin(), preprocessed_input.end(), randomFloatGenerator);
        bool valid_frame = preprocess(raw_input,preprocessed_input);

        if(valid_frame){

            
            Ort::Value input_tensor = Ort::Value::CreateTensor<float>(
                memory_info,
                preprocessed_input.data(),
                preprocessed_input.size(),
                input_shape.data(),
                input_shape.size()
            );

            std::cout<<"Running Inference.."<<std::endl;
            auto output_tensors = session.Run(
                run_options,
                input_names,
                &input_tensor,
                1,  // one input tensor
                output_names,
                1   // one output tensor
            );

            std::cout<<"Inferece completed!"<<std::endl;

            float* output_data = output_tensors[0].GetTensorMutableData<float>();
            
            std::cout << "Output sample: " << output_data[0] << std::endl;
            printOutputShape(output_tensors);
        }
    }
    return 0;
}