# filename = "C:/Users/Benoit/Documents/02_DATA/WAV files test Classification/A1_210918_01_1_2.WAV"
# filename = "C:\\Users\\Benoit\\Documents\\02_DATA\\EMIKA\\XTO_WAV_LOOPS\\XTO_DRUM_LPS\\XTO_125_Paradyse_Drums_Top.wav"
# filename = "C:\\Users\\Benoit\\Documents\\02_DATA\\EMIKA\\XTO_WAV_LOOPS\\XTO_DRUM_LPS\\XTO_138_Promise_Drums_Top.wav"

filename  = "C:/Users/Benoit/Documents/Auto-Tempo-Detection/libs/BeatNet/onnx/test/samples/108bpm.wav"
# filename  = "C:/Users/Benoit/Documents/Auto-Tempo-Detection/libs/BeatNet/onnx/test/samples/120bpm.wav"
# filename  = "C:/Users/Benoit/Documents/Auto-Tempo-Detection/libs/BeatNet/onnx/test/samples/128.wav"

#import librosa
#filename = librosa.example('nutcracker')

from BeatNet.BeatNet import BeatNet

#estimator = BeatNet(1, mode='realtime', inference_model='PF', plot=['beat_particles'], thread=False)
#estimator = BeatNet(1, mode='realtime', inference_model='PF', plot=['downbeat_particles'], thread=False)
# estimator = BeatNet(1, mode='realtime', inference_model='PF', plot=['activations', 'downbeat_particles', 'beat_particles'], thread=False)


# Using Particle Filtering online inference to infer beat/downbeats
estimator = BeatNet(1, mode='realtime', inference_model='PF', plot=[], thread=False) # DOES NOT WORK WELL

# Using Dynamic Bayesian Network Inference (non-causal)    
#estimator = BeatNet(1, mode='offline', inference_model='DBN', plot=[], thread=False) # WORKS WELL


#estimator = BeatNet(1, mode='offline', inference_model='DBN', plot=[], thread=False)
Output = estimator.process(filename)


# -----------  plot audio and beats detected
import librosa
import matplotlib.pyplot as plt
import numpy as np

time_detected_beats = Output[:,0]

audio_vec, sr = librosa.load(filename)  # reading the data
time_vec = np.arange(len(audio_vec))/sr

fig, ax = plt.subplots()
plt.plot(time_vec, audio_vec)

ax.set_title(filename)
ax.set(ylim = (-1.5, 1.5))
ax.set_xticks(range(30))

for t0 in time_detected_beats:
    ax.plot([t0,t0], [-1.2,1.2], "-or")

#-----------------------------      

N = len(Output)

diff = Output[1:N,0] - Output[0:N-1,0]
import matplotlib.pyplot as plt
plt.figure()
plt.plot(60 / diff)

# histogramme
fhisto, axhisto = plt.subplots()

axhisto.hist(60 / diff, bins = 30)


if False:
    import pyaudio
    import wave
    
    
    # Set chunk size of 1024 samples per data frame
    chunk = 1024  
    
    # Open the sound file 
    wf = wave.open(filename, 'rb')
    
    # Create an interface to PortAudio
    p = pyaudio.PyAudio()
    
    # Open a .Stream object to write the WAV file to
    # 'output = True' indicates that the sound will be played rather than recorded
    stream = p.open(format = p.get_format_from_width(wf.getsampwidth()),
                    channels = wf.getnchannels(),
                    rate = wf.getframerate(),
                    output = True)
    
    # Read data in chunks
    data = wf.readframes(chunk)
    
    # Play the sound by writing the audio data to the stream
    while data != '':
        stream.write(data)
        data = wf.readframes(chunk)
    
    while data != '':
        stream.write(data)
        data = wf.readframes(chunk)
    
    # Close and terminate the stream
    stream.close()
    p.terminate()