import sys
import os
sys.path.append(os.path.abspath(os.path.join(os.getcwd(), "..", "..", "src")))
sys.path.append(os.path.abspath(os.path.join(os.getcwd(), "..", "..", "src", "BeatNet")))
print(sys.path)
import numpy as np
from BeatNet.BeatNet import BeatNet
import librosa

# config 
samplerate = 22050 
buffersize = 512

wavDir = "samples"

# class that replaces the microphone input to feed buffers of audio data instead    
class BufferStream:
    '''
    Class that replaces the pyaudio.PyAudio().open()->_Stream that is used from BeatNet in stream mode to read the microphone input.
    @see BeatNet.py->line 87
    '''
    def __init__(self, audiopath):
        self.audio, _ = librosa.load(audiopath, sr=samplerate, mono=True, dtype=np.float32)
        self.active = True
        self.pos = 0
        self.numSamples = len(self.audio) 

    def getNumSamples(self):
        return self.numSamples
    
    def read(self, size):
        # if last buffer
        if (self.numSamples < (self.pos+1)*buffersize):
            self.active = False            

        # read buffer
        buffer = self.audio[self.pos*buffersize : (self.pos+1)*buffersize]
        self.pos += 1
        return buffer.tobytes()

    def is_active(self):
        return self.active

filelist = os.listdir(wavDir)
for audiosample in filelist:
    import pdb; pdb.set_trace()

    audiopath = os.path.join(wavDir, audiosample)
    
    print(f"Processing {audiopath}...")

    # init beatnet model
    # estimator = BeatNet(1, mode='stream', inference_model='PF', thread=False)
    # customBufferStream = BufferStream(audiopath)
    # estimator.stream = customBufferStream
    estimator = BeatNet(1, mode='realtime', inference_model='PF', plot=['activations', 'downbeat_particles', 'beat_particles'], thread=False)
    output = estimator.process(audiopath)  # read buffers internally until is_active() is falseaudiopath
    # output = estimator.process()  # read buffers internally until is_active() is false

    # store results
    outputFile = os.path.join(os.getcwd(), "results" ,os.path.basename(audiopath).split(".wav")[0] + "_py")
    print(output)
    if len(output)>0:
        print(len(output)/ (output[-1][0]- output[0][0])*60*2)
    with open(outputFile, "w") as f:
        for beat in output:
            # import pdb; pdb.set_trace()
            f.write(f"{beat[0],beat[1]}\n")

    # print(f"Finished processing {audiopath} ({customBufferStream.getNumSamples()} samples)")
