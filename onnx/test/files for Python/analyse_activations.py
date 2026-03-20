
import matplotlib.pyplot as plt
import numpy as np
import re
import librosa




def getdatafromfile(filepath):
    time_vec = list()
    beatActivations_vec = list()
    downbeatActivations_vec = list()

    with open(filepath, "r") as file:
        for line in file:
            newline = line[:-1] # remove '\n'
            time, beat, downbeat = re.split(" ", newline)

            time_vec.append(float(time))
            beatActivations_vec.append(float(beat))
            downbeatActivations_vec.append(float(downbeat))

            # time, beat, downbeat = newline
            # print(time)

    return np.array(time_vec), np.array(beatActivations_vec), np.array(downbeatActivations_vec)

def getFeaturesdatafromfile(filepath):
    allFeatures = list()

    with open(filepath, "r") as file:
        for line in file:
            newline = line[:-1] #remove '\n'
            valuesList = newline.split(" ")

            for k, valuestr in enumerate(valuesList):
                valuesList[k] = float(valuestr)

            allFeatures.append(valuesList.copy())


    return np.array(allFeatures)


# bpmvalue = 108
# bpmvalue = 120
# bpmvalue = 128

results_dir = R"C:/Users/Benoit/Documents/Auto-Tempo-Detection/libs/BeatNet/onnx/test/results/"
samples_dir = R"C:/Users/Benoit/Documents/Auto-Tempo-Detection/libs/BeatNet/onnx/test/samples/"

for bpmvalue in [108, 120, 128]:
    file_python = results_dir + "{}bpm_beat_activations_py".format(bpmvalue)
    file_cpp = results_dir + "{}bpm_beat_activations_cpp".format(bpmvalue)

    fileaudio = samples_dir + "{}bpm.wav".format(bpmvalue)
    (audio_vec, sr) = librosa.load(fileaudio)  # reading the data
    time_vec = np.arange(len(audio_vec))/sr
    tmax = time_vec[-1]

    (tpy, bpy, dbpy) = getdatafromfile(file_python)
    (tcpp, bcpp, dbcpp) = getdatafromfile(file_cpp)

    #  beat and downbeat activations
    fig, (ax1, ax2, ax3) = plt.subplots(3, 1)

    ax1.plot(time_vec, audio_vec)
 
    ax1.plot(tpy, bpy,'g')
    ax1.plot(tcpp, bcpp, 'y')

    ax2.plot(tpy, bpy,'g')
    ax2.plot(tcpp, bcpp, 'y')

    ax3.plot(tpy, dbpy, 'b')
    ax3.plot(tcpp, dbcpp, 'r')

    # features
    file_features_cpp = results_dir + "{}bpm_features_cpp".format(bpmvalue)
    allfeats_cpp = getFeaturesdatafromfile(file_features_cpp)

    file_features_python = results_dir + "{}bpm_features_py".format(bpmvalue)
    allfeats_python = getFeaturesdatafromfile(file_features_python)

    fig0, (ax_cpp, ax_py) = plt.subplots(2, 1)
    ax_cpp.imshow(allfeats_cpp.T)
    ax_cpp.invert_yaxis()
    ax_cpp.set_title("features from cpp")

    ax_py.imshow(allfeats_python.T)
    ax_py.invert_yaxis()
    ax_py.set_title("features from python")

    
    fig_1feat, ax_1feat = plt.subplots()

    index = 30 # change the index to compare different frame features
    ax_1feat.plot(allfeats_cpp[index,:],'y')
    ax_1feat.plot(allfeats_python[index,:],'g')

    plt.show()

