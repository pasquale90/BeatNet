import sys
import os
sys.path.append(os.path.abspath(os.path.join(os.getcwd(), "..", "..", "src")))
print(sys.path)
import numpy as np
from BeatNet.BeatNet import BeatNet
import librosa

# config 
samplerate = 22050 
buffersize = 512

# class that replaces the microphone input to feed buffers of audio data instead    
class BufferStream:
    '''
    Class that replaces the pyaudio.PyAudio().open()->_Stream that is used from BeatNet in stream mode to read the microphone input.
    @see BeatNet.py->line 87
    '''
    def __init__(self, audiopath):
        self.audio, _ = librosa.load(audiopath, sr=samplerate, mono=True)
        self.is_active = True
        # use a position variable
        # use a output file variable to store results

    def read(self, size):
        pass
        # if last buffer
            # set self.is_active to false 
            # store results
        # read buffer
        # zero pad if needed
        # ++position

    def is_active(self):
        return self.active

filelist = os.listdir("samples")
for audiosample in filelist:
    audiopath = os.path.join("samples", audiosample)
    
    print(f"Processing {audiopath}...")

    # init beatnet model
    estimator = BeatNet(1, mode='stream', inference_model='PF', thread=False)
    estimator.stream = BufferStream(audiopath)

    output = estimator.process()  # read buffers internally until is_active() is false