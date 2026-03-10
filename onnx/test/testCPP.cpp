#include <iostream>
#include "AudioFile/AudioFile.h" //--> https://github.com/adamstark/AudioFile
#include <string>
#include <filesystem>
#include "BeatNet.h"
#include <utility>

static int buffersize = 2293; // in BeatNet at 22050 samples/sec
typedef float bitQuantization; 

namespace fs = std::filesystem;

const fs::path currentFilePath = fs::path(__FILE__).parent_path();
const fs::path samplesPath = currentFilePath / "samples";
const fs::path resultsPath = currentFilePath / "results";

int main() {

	std::cout<<"Testing cpp implementation of BeatNet"<<std::endl;

	// iterate over files
	for (const auto& audioSample : fs::directory_iterator(samplesPath))
	{
		std::string audiopath = audioSample.path().string();
		std::cout << "Processing " << audiopath << std::endl;

		// results filenames for output file
		const std::string fileName = fs::path(audioSample).stem().string();
		const fs::path outputPath = resultsPath;
		const fs::path outputFilePath = outputPath / std::string(fileName + "_cpp");

		// load file
		AudioFile<bitQuantization> audioFile;
		audioFile.load(audiopath);		

		// values of parameters at the samplerate of the loaded file
		const int numSamples = audioFile.getNumSamplesPerChannel();
		const float sr_inputWavfile = static_cast<float>(audioFile.getSampleRate());
		const int sr_Ratio = static_cast<int>(sr_inputWavfile / SR_BEATNET); // 44100 / 22050 = 2 
		const float dt = 1.0f / sr_inputWavfile;

		const int buffersize_current = buffersize * sr_Ratio;
		const int FRAME_LENGTH_current = FRAME_LENGTH * sr_Ratio;
		const int HOP_SIZE_current = HOP_SIZE * sr_Ratio;

		// initialize BeatNet (note:processes float buffers)
		BeatNet model;
		model.setup(sr_inputWavfile, buffersize_current);

		// predict beats and downbeats
		std::vector<std::pair<float, int>> beatPositions;
		std::vector<std::pair<float, int>> downBeatPositions;

		std::vector<float> time_vec;
		std::vector<float> beatActivations_vec;
		std::vector<float> downbeatActivations_vec;

		for (int idx=0; idx + buffersize_current < numSamples; idx += HOP_SIZE_current)
		{
			std::vector<float> audioBlockInput(audioFile.samples[0].begin() + idx,
										  audioFile.samples[0].begin() + idx + buffersize_current);
			std::vector<float> output;

			if (model.process(audioBlockInput, output))
			{
				// output[0] : downbeat
				// output[1] : beat
				// output[2] : no beat

				float time_index = (float)idx + (-(FRAME_LENGTH_current / 2) + (3 * HOP_SIZE_current));
				float time_seconds = time_index * dt;

				time_vec.push_back(time_seconds);
				beatActivations_vec.push_back(output[1]);
				//downbeatActivations_vec.push_back(1.0f - output[2]);
				downbeatActivations_vec.push_back(output[0]);


				int indexMaxProbability = std::max_element(output.begin(), output.end()) - output.begin();
				switch (indexMaxProbability)
				{
					case 0: // downbeat
						downBeatPositions.push_back({ time_seconds, 0 }); //
						break;
					case 1: // beat
						beatPositions.push_back({ time_seconds, 1 });
						break;
				}
			}
		}

		// write beat time positions to file, one per line
		if (beatPositions.size() > 0)
		{			
			std::ofstream outFile(outputFilePath);
			for (const auto& [time, beatValue] : beatPositions)
			{
				outFile << time << "," << beatValue << std::endl;
			}
			outFile.close();
		}

		// write beat activations 
		if (beatActivations_vec.size() > 0)
		{
			std::ofstream outFile(outputPath / std::string(fileName + "_beatActivations_cpp"));
			for (int i= 0; i < beatActivations_vec.size(); ++i)
			{
				outFile << time_vec[i] << " " << beatActivations_vec[i] << " " << downbeatActivations_vec[i] << std::endl;
			}
			outFile.close();
		}

		// write features extracted
		const auto& features = model.features_extracted;
		if (features.size() > 0)
		{
			std::ofstream outFile(outputPath / std::string(fileName + "_features_cpp"));
			for (const auto& feat_vec: features)
			{
				int lastindex = feat_vec.size() - 1;
				for (int i = 0; i < lastindex; ++i)
					outFile << feat_vec[i] << " ";

				// print the last value with '\n'
				outFile << feat_vec[lastindex] << std::endl;
			}
			outFile.close();
		}

		std::cout << "Finished processing " << audiopath << " (" << numSamples << " samples)" << std::endl;
	}
	return 0;
}
