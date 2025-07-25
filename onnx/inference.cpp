#include "onnxruntime_cxx_api.h"
#include <iostream>
#include <string>
#include <filesystem>
#include <random>

std::string modelPath("beatnet_bda.onnx");

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

    std::cout << "Creating tensor..." << std::endl;
    std::vector<int64_t> input_shape = {1,1,272};
    std::vector<float> input_tensor_values( 1 * 1 * 272); // store linearly in memory
    std::generate(input_tensor_values.begin(), input_tensor_values.end(), randomFloatGenerator);

    Ort::MemoryInfo memory_info = Ort::MemoryInfo::CreateCpu(OrtArenaAllocator, OrtMemTypeDefault);
    Ort::Value input_tensor = Ort::Value::CreateTensor<float>(
        memory_info,
        input_tensor_values.data(),
        input_tensor_values.size(),
        input_shape.data(),
        input_shape.size()
    );

    std::cout<<"Running Inference.."<<std::endl;
    Ort::RunOptions run_options{nullptr};
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

    return 0;
}