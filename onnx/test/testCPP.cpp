#include <iostream>
#include "AudioFile/AudioFile.h" //--> https://github.com/adamstark/AudioFile
#include <string>
#include <filesystem>
#include "BeatNet.h"
#include <utility>

static int samplerate = 44100;
static int buffersize = 2293 * 2;
typedef float bitQuantization; 
namespace fs = std::filesystem;

fs::path wavDir = fs::current_path().parent_path().parent_path().parent_path() / "samples";


int main() {

	std::cout<<"Testing cpp implementation of BeatNet"<<std::endl;
	std::cout << wavDir.string() << std::endl;

	// iterate over files
	for(const auto & audioSample : std::filesystem::directory_iterator(wavDir)) 
	{

		std::string audiopath = audioSample.path().string();
		std::cout << "Processing " << audiopath << std::endl;

		// results filenames for output file
		const std::string fileName = std::filesystem::path(audioSample).stem().string();
		const std::filesystem::path outputPath = std::filesystem::current_path();
		const std::string outputFilePath = outputPath.string() + "\\" + fileName + "_cpp";

		// load file
		AudioFile<bitQuantization> audioFile;
		audioFile.load(audiopath);
		const double sr_inputWavfile = static_cast<double>(audioFile.getSampleRate());

		// initialize BeatNet (note:processes float buffers)
		BeatNet model;
		model.setup(sr_inputWavfile, buffersize);
		
		int numSamples = audioFile.getNumSamplesPerChannel();
		
		std::vector<float> time_beats;
		std::vector<float> output0; 
		std::vector<float> output1;
		std::cout << "Num samples: " << numSamples << std::endl;
		std::vector<std::pair<int, int>> beatPositions;
		for (int idx=0; idx + (buffersize) < numSamples; idx += (441 * 2))
		{
			// predict beats
			std::vector<float> audioInput(audioFile.samples[0].begin() + idx,
										  audioFile.samples[0].begin() + idx + buffersize);
			std::vector<float> output;
			
			if (model.process(audioInput, output))
			{
				output0.push_back(output[0]);
				output1.push_back(output[1]);

				int argmax = std::max_element(output.begin(), output.end()) - output.begin();

				if (argmax != 2)// if beat
				{
					beatPositions.push_back({ idx, argmax });
					time_beats.push_back(((float)idx) + 2 * (-705 + (3 * 441)));
				}
			}
			
				// write beat positions to file, one per line
				std::ofstream outFile(outputFilePath);
				for (const auto& beat : beatPositions)
				{
				outFile << beat.first << "," << beat.second << std::endl;
			}
			outFile.close();
		}
		std::cout << "Finished processing " << audiopath << " (" << numSamples << " samples)" << std::endl;
	}
	return 0;
}