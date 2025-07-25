# requirements

- onnxruntime 
Download binaries from official repo ([here](https://github.com/microsoft/onnxruntime/releases/tag/v1.22.1)) and export in this directory under `onnxruntime` dir.
- python, and a python env containing the following packages:
python=3.9
numpy=2.0.2
scipy=1.10.1
torch=2.7.1
onnx=1.12.0
onnxruntime=1.19.2
librosa=0.10.2
madmom=0.16.1

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

## Build inference ONNX in C++
```
cd BeatNet/onnx
cmake -B build
cmake --build build
```
## Test inference ONNX in C++
Before testing, copy the library file from the `BeatNet/onnx/onnxruntime/lib` dir into the `build/Debug` dir.
```
build/Debug/beatnet_infer
```
The output should be something like:

```
$ BeatNet\onnx> build\Debug\beatnet_infer.exe
Starting inference...
Loading model: beatnet_bda.onnx
Fetching input/output names...
Input name: input
Output name: output
Creating tensor...
Running Inference..
Inferece completed!
Output sample: 0.800519
Output shape: [1, 3, 1]
```

## Integration related actions
ref : https://arxiv.org/pdf/2108.03576
# replicate pre-processing 
- resample to 22050 samples per second
- extract 272-d filterbank response vector for each frame
- tensorize input data
- ...??
# integrate inference
- integrate inference into AutoTempoDetection's callback function
# replicate post-processing
- ...??
- detensorize output data



