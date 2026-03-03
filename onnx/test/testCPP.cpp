#include <iostream>
#include "AudioFile/AudioFile.h" //--> https://github.com/adamstark/AudioFile
#include <string>
#include <filesystem>
#include "BeatNet.h"
#include <utility>

//static int samplerate = 44100;
static int buffersize = 2293; // at 22050 samples/sec
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

		// values of parameters at the samplerate of the loaded file
		const float sr_inputWavfile = static_cast<float>(audioFile.getSampleRate());
		const int sr_Ratio = static_cast<int>(sr_inputWavfile / SR_BEATNET); // 44100 / 22050 = 2 
		const float dt = 1.0f / sr_inputWavfile;

		const int buffersize_current = buffersize * sr_Ratio;
		const int FRAME_LENGTH_current = FRAME_LENGTH * sr_Ratio;
		const int HOP_SIZE_current = HOP_SIZE * sr_Ratio;

		// initialize BeatNet (note:processes float buffers)
		BeatNet model;
		model.setup(sr_inputWavfile, buffersize_current);

		int numSamples = audioFile.getNumSamplesPerChannel();
		
		std::vector<float> time_beats;
		std::vector<float> output0; 
		std::vector<float> output1;
		std::cout << "Num samples: " << numSamples << std::endl;

		std::vector<std::pair<float, float>> beatPositions;
		std::vector<std::pair<float, float>> downBeatPositions;		

		for (int idx=0; idx + (buffersize_current) < numSamples; idx += (HOP_SIZE_current))
		{
			// predict beats
			std::vector<float> audioInput(audioFile.samples[0].begin() + idx,
										  audioFile.samples[0].begin() + idx + (buffersize_current));
			std::vector<float> output;
			
			if (model.process(audioInput, output))
			{
				// output[0] : downbeat
				// output[1] : beat
				// output[2] : no beat
				output0.push_back(output[0]);
				output1.push_back(output[1]);

				int indexMaxProbability = std::max_element(output.begin(), output.end()) - output.begin();

				float time_index = (float)idx + (-(FRAME_LENGTH_current / 2) + (3 * HOP_SIZE_current));
				switch (indexMaxProbability)
				{
					case 0: // downbeat
						time_beats.push_back(time_index);
						downBeatPositions.push_back({ time_index * dt, 0.3f });
						break;
					case 1: // beat
						time_beats.push_back(time_index);
						beatPositions.push_back({ time_index * dt, 0.31f }); 
						break;
				}

			}
		}

		// write beat positions to file, one per line
		if (beatPositions.size() > 0)
		{			
			std::ofstream outFile(outputFilePath);
			for (const auto& beat : beatPositions)
			{
				outFile << beat.first << "," << 1 << std::endl;
			}
			outFile.close();
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
		for (float i = 0.0f; i < plot_output0.size(); ++i)
		{
			auto time = t0 + ((FRAME_LENGTH_current) + i * (HOP_SIZE_current)) * dt;
			plot_output0[i] = { time , output0[i] };
			plot_output1[i] = { time , output1[i] };
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
