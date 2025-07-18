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

## Test inference ONNX in C++




