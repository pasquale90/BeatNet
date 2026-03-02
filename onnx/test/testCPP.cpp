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
	for (const auto& audioSample : std::filesystem::directory_iterator(wavDir))
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
			if (beatPositions.size() > 0)	
			{
				// write beat positions to file, one per line
				std::ofstream outFile(outputFilePath);
				for (const auto& beat : beatPositions)
				{
					outFile << beat.first << "," << beat.second << std::endl;
				}
				outFile.close();
			}
		}


		if (!time_beats.empty())
		{
			std::vector<float> difference(time_beats.size() - 1);
			std::vector<float> bpm(time_beats.size() - 1);

			std::transform(time_beats.begin() + 1, time_beats.end(),
				time_beats.begin(), difference.begin(), std::minus());

			for (int i = 0; i < difference.size(); ++i)
			{
				bpm[i] = 60.0f / difference[i];
			}

			// std::cout << std::endl;
			// for (auto& val : time_beats)
			// {
			// 	std::cout << val << std::endl;
			// }
			// std::cout << std::endl;
		}
		auto signal = audioFile.samples[0];
		std::vector<float> times_plot(signal.size(), std::nan("1"));

		for (auto& i : time_beats)
		{
			times_plot[(int) i] = 1.5f;
		}
		//signal.erase(signal.begin() + signal.size() /4, signal.end());
		//times_plot.erase(times_plot.begin() + times_plot.size() / 4, times_plot.end());

		std::vector<std::pair<float, float>> plot_output0(output0.size());
		std::vector<std::pair<float, float>> plot_output1(output0.size());

		float t0 = 0.0f;
		float dt = 1.0f / static_cast<float>(samplerate);
		for (float i = 0.0f; i < plot_output0.size(); ++i)
		{
		plot_output0[i] = { t0 + (1441 * 2)*dt + (i *(441*2)* dt) , output0[i] };
		plot_output1[i] = { t0 + (1441 * 2)*dt + (i *(441*2)* dt) , output1[i] };
		}


		std::vector<float> time_vec(signal.size());
		{
			float index = 0.0f;
			std::generate(time_vec.begin(), time_vec.end(), [t0, dt, &index]() {return t0 + (index++ * dt); });
		}
		{
			int index = 0;
			auto end = std::remove_if(signal.begin(), signal.end(), [&index](float x) {if (index > 9) index = 0; return index++ > 0; });
			signal.erase(end, signal.end());
		}

		{
			int index = 0;
			auto end = std::remove_if(time_vec.begin(), time_vec.end(), [&index](float x) {if (index > 9) index = 0; return index++ > 0; });
			time_vec.erase(end, time_vec.end());
		}

		std::vector<std::pair<float, float>> plot_signal(time_vec.size());
		for (float i = 0.0f; i < time_vec.size(); ++i)
		{
			plot_signal[i] = { time_vec[i] , signal[i] };
		}

		int stop = 0;

		std::cout << "Finished processing " << audiopath << " (" << numSamples << " samples)" << std::endl;
	}
	return 0;
}
