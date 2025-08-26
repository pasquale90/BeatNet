# requirements

- onnxruntime 
Download binaries from official repo ([here](https://github.com/microsoft/onnxruntime/releases/tag/v1.22.1)) and export in this directory under `onnxruntime` dir.
- python, and a python env containing the following packages:
python=3.9
numpy=2.0
scipy=1.10
torch=2.7
onnx=1.12
onnxruntime=1.19
librosa=0.10
madmom=0.16

# export the model to onnx

```
cd BeatNet/onnx
python exportModel.py
```

# test inference with ONNX in Python
```
cd BeatNet/onnx
python testModel.py
```

## Build library
```
cd BeatNet/onnx
cmake -B build
cmake --build build
```
## Test inference ONNX in C++
To create an executable for testing the library, compile with `BUILD_APP` enabled:

```
cmake -B build -D BUILD_APP=ON
cmake --build build
```

Then run with:

```
build/Debug/beatnet_infer
```

The indicative console output should print info regarding the output of the model after passing random data as input to the model.

```
$ BeatNet\onnx> build\Debug\beatnet_infer.exe
...
Output shape: [1, 3, 1]
BeatNet Output: [-0.523651 -0.572624 1.00063 ]
```

## Integration related actions
ref : https://arxiv.org/pdf/2108.03576
- replicate pre-processing 
    - resample to 22050 samples per second
    - extract 272-d filterbank response vector for each frame
    - tensorize input data
    - ...??
- integrate inference
    - integrate inference into AutoTempoDetection's callback function
- replicate post-processing
    - ...??
    - detensorize output data



