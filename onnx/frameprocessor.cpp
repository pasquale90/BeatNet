#include "frameprocessor.h"
#include <algorithm>
#include <stdexcept>
#include "iostream"

FramedSignalProcessor::FramedSignalProcessor(const std::vector<float>& inputSignal, int nFrames, int frameSize, int hopSize)
	: original_signal(inputSignal),
	  nFrames(nFrames),
	  frameSize(frameSize),
	  hopSize(hopSize)
{
	int nMax = ((nFrames -1) * hopSize) + frameSize;
	padded_signal.assign(nMax, 0.0f);

	auto b = original_signal.begin();
	auto e = original_signal.end();
	//if (frameSize / 2 + original_signal.size() > nMax)
	//{
	//	int nSamplesToRemove = (frameSize / 2 + original_signal.size()) - nMax;
	//	e = original_signal.end() - nSamplesToRemove;
	//}

	//std::copy(b, e, padded_signal.begin() + frameSize / 2);

	{
		int i = frameSize / 2;
		std::copy_if(b, e, padded_signal.begin() + frameSize / 2, 
			[&i, nMax, frameSize](float x) 
			{
				//if (i< nMax)
				//std::cout << i << std::endl;

				return i++ < (nMax); 
			});
	}

	int i0 = 0;
	for (int iFrame = 0; iFrame < nFrames; ++iFrame)
	{
		auto start = padded_signal.begin() + i0;
		std::vector<float> signal(start,  start + frameSize);
		frames.push_back(signal);

		i0 += hopSize;
	}
}

FramedSignalProcessor::~FramedSignalProcessor()
{

}

std::vector<float> FramedSignalProcessor::operator[](int i)
{
	return frames.at(i);
}

std::vector<float> FramedSignalProcessor::getOriginalSignal()
{
	return original_signal;
}

int FramedSignalProcessor::get_nFrames()
{
	return nFrames;
}
//FramedSignalProcessor::FramedSignalProcessor(int frameSize, int hopSize)
//    : frame_size(frameSize),
//      hop_size(hopSize),
//      ring_size(frame_size*1.1),            // make it a litle bit longer
//      write_pos(0),
//      total_samples_written(0),
//      ring_buffer(ring_size, 0.0f) {
//
//
//}

//void FramedSignalProcessor::reset() {
//    std::fill(ring_buffer.begin(), ring_buffer.end(), 0.0f);
//    write_pos = 0;
//    total_samples_written = 0;
//}

//bool FramedSignalProcessor::process(const std::vector<float>& input, std::vector<float>& frame_out) {
//
//    for (float sample : input) {
//        ring_buffer[write_pos] = sample;
//        write_pos = (write_pos + 1) % ring_size;
//        total_samples_written++;
//    }
//
//    if (total_samples_written>=frame_size)
//    {
//        frame_out.resize(frame_size);
//        for (int i = 0; i < frame_size; ++i) {
//            int index = (write_pos + ring_size - frame_size + i) % ring_size;
//            frame_out[i] = ring_buffer[index];
//        }
//        return true;
//    }
//    else return false;
//}