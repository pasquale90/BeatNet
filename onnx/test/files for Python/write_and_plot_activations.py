

import os
import librosa

from BeatNet.BeatNet import BeatNet


import numpy as np
import matplotlib.pyplot as plt

def plot_results(filename, Output, beat_activationsList, downbeat_activationsList, time_activationsList, feats_all, plt):

        
    fig0, ax0 = plt.subplots()
    ax0.imshow(feats_all.T, extent=[0, 5, 1, 0])
    ax0.invert_yaxis()
    ax0.set_title("features from python")

    # fig_activations, ax_activations = plt.subplots()
    # plt.plot(time_activationsList, beat_activationsList)
    # plt.plot(time_activationsList, downbeat_activationsList)

    # -------------------------------------------------------------
    time_detected_beats = list()
    time_detected_downbeats = list()
    for time, val in Output:
        if val == 2:
            time_detected_beats.append(time)

        if val == 1:
            time_detected_downbeats.append(time)


    # time_detected_beats = Output[:,0]

    audio_vec, sr = librosa.load(filename)  # reading the data
    time_vec = np.arange(len(audio_vec))/sr

    fig, ax = plt.subplots()
    plt.plot(time_vec, audio_vec)

    ax.set_title(filename)
    ax.set(ylim = (-1.5, 1.5))
    ax.set_xticks(range(30))

    for t0 in time_detected_beats:
        t0  = t0 +0.04
        ax.plot([t0,t0], [-1.2,1.2], "-oy")

    for t0 in time_detected_downbeats:
        ax.plot([t0,t0], [-1.2,1.2], "-or")

    #-----------------------------  
    plt.plot(time_activationsList + 0.07, beat_activationsList, "y")
    plt.plot(time_activationsList + 0.07, downbeat_activationsList, "r")    
    #-----------------------------  

    N = len(Output)

    diff = Output[1:N,0] - Output[0:N-1,0]
    import matplotlib.pyplot as plt
    plt.figure()
    plt.plot(60 / diff)

    # histogramme
    fhisto, axhisto = plt.subplots()

    axhisto.hist(60 / diff, bins = 30)

def store_results(filename, Output, beat_activationsList, downbeat_activationsList, time_activationsList, feats_all):

    outputFileBeatActivations = os.path.join(os.getcwd(), "results" ,os.path.basename(filename).split(".wav")[0] + "_beat_activations_py")

    filenameOutput_beat = os.path.basename(filename).split(".wav")[0] + "_beat_activations_py"
    filepathResults = os.path.abspath(R"C:/Users/Benoit/Documents/Auto-Tempo-Detection/libs/BeatNet/onnx/test/results")
    
    filepath = os.path.join(filepathResults ,filenameOutput_beat)
    with open(filepath, "w") as f:
        for time, beat, downbeat in zip(time_activationsList.tolist(), beat_activationsList.tolist(), downbeat_activationsList.tolist()):
            # import pdb; pdb.set_trace()
            f.write("{0:2.4f} {1:2.4f} {2:2.4f}\n".format(time, beat, downbeat))

    # write features

    filenameOutput_features = os.path.basename(filename).split(".wav")[0] + "_features_py"
    filepath = os.path.join(filepathResults,filenameOutput_features)
    with open(filepath, "w") as f:
        for feature_vec in feats_all:

            lastindex = len(feature_vec) - 1 
            for i in range(lastindex):
                f.write(str(feature_vec[i]) +  " ")

            # write last value with  '\n'
            f.write(str(feature_vec[lastindex]) + "\n")


# filename  = "C:/Users/Benoit/Documents/Auto-Tempo-Detection/libs/BeatNet/onnx/test/samples/108bpm.wav"
# filename  = "C:/Users/Benoit/Documents/Auto-Tempo-Detection/libs/BeatNet/onnx/test/samples/120bpm.wav"


# Using Particle Filtering online inference to infer beat/downbeats
estimator = BeatNet(1, mode='realtime', inference_model='PF', plot=[], thread=False) 

samplesFolderPath = os.path.abspath(R"C:/Users/Benoit/Documents/Auto-Tempo-Detection/libs/BeatNet/onnx/test/samples")

for file in os.listdir(samplesFolderPath):
    if file.endswith(".wav"):

        filepath = os.path.join(samplesFolderPath, file)
        print(filepath)
        Output, beat_activationsList, downbeat_activationsList, time_activationsList, feats_all = estimator.process(filepath)

        store_results(filepath, Output, beat_activationsList, downbeat_activationsList, time_activationsList, feats_all)

        #plot_results(filepath, Output, beat_activationsList, downbeat_activationsList, time_activationsList, feats_all, plt)

        #plt.show()


