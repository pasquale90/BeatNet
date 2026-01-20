#include <iostream>
#include "AudioFile/AudioFile.h" //--> https://github.com/adamstark/AudioFile
#include <string>
#include <filesystem>
#include "BeatNet.h"

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
		
			// read audio in chunks
			// predict beats
			// store beats into file
	}
	return 0;
}