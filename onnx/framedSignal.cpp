#include "framedSignal.h"
#include <algorithm>
#include <stdexcept>
#include "iostream"

FramedSignal::FramedSignal(const std::vector<float>& inputSignal, int nFrames, int frameSize, int hopSize)
	: original_signal(inputSignal),
	  nFrames(nFrames),
	  frameSize(frameSize),
	  hopSize(hopSize)
{
	int nMax = ((nFrames -1) * hopSize) + frameSize;
	padded_signal.assign(nMax, 0.0f);

	{
		auto s0 = original_signal.begin();
		auto sEnd = original_signal.end();
		auto destination = padded_signal.begin() + frameSize / 2;

		int i = frameSize / 2;

		std::copy_if(s0, sEnd, destination, 
			[&i, nMax](float x) 
			{
				return i++ < nMax; 
			});
	}

	for (int iFrame = 0, index = 0; iFrame < nFrames; iFrame++, index += hopSize)
	{
		auto i0 = padded_signal.begin() + index;

		std::vector<float> signal(i0,  i0 + frameSize);
		frames.push_back(signal);
	}
}

FramedSignal::~FramedSignal()
{

}

std::vector<float> FramedSignal::operator[](int i)
{
	return frames.at(i);
}

std::vector<float> FramedSignal::getOriginalSignal()
{
	return original_signal;
}

int FramedSignal::get_nFrames()
{
	return nFrames;
}
