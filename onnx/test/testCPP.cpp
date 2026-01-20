#include <iostream>
#include "AudioFile/AudioFile.h" //--> https://github.com/adamstark/AudioFile
#include <string>
#include <filesystem>
#include "BeatNet.h"
#include <utility>

static int samplerate = 22050;
static int buffersize = 512;
typedef float bitQuantization; 
static const std::string wavDir = "samples";


int main() {

	std::cout<<"Testing cpp implementation of BeatNet"<<std::endl;

	// iterate over files
	for(const auto & audioSample : std::filesystem::directory_iterator(wavDir)) 
	{
		
		std::string audiopath = (std::filesystem::current_path() / audioSample).string();
		std::cout<<"Processing "<<audiopath<<std::endl;
		
		// load file
		AudioFile<bitQuantization> audioFile;
		audioFile.load(audiopath);

		// initialize BeatNet (note:processes float buffers)
		BeatNet model;
		model.setup(static_cast<double>(samplerate), buffersize);
		
		int numSamples = audioFile.getNumSamplesPerChannel();
		std::cout<<"Num samples: "<<numSamples<<std::endl;
		std::vector<std::pair<int,int>> beatPositions;
		for (int idx=0; idx < numSamples; ++idx)
		{

			// read audio in chunks
			if (numSamples < (idx+1) * buffersize)
			{
				break;
			}

			// predict beats
			std::vector<float> audioInput(audioFile.samples[0].begin() + idx*buffersize,
									audioFile.samples[0].begin() + (idx+1)*buffersize);
			std::vector<float> output;

			if (model.process(audioInput, output))
			{
				int argmax = std::max_element(output.begin(), output.end()) - output.begin();

				if (argmax != 2)// if beat
				{
					beatPositions.push_back({idx, argmax});
				}
			}
			
			// store beats into file
			std::string fileName = std::filesystem::path(audioSample).stem().string();
			std::string outputPath = "results/" + fileName + "_cpp";

			// write beat positions to file, one per line
			std::ofstream outFile(outputPath);
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