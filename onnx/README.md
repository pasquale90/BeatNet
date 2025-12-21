
# BeatNet ONNX Implementation

This is the onnx implementation of the BeatNet model. 
reference : [BeatNet paper](https://arxiv.org/pdf/2108.03576)

# About 

The work involves porting the Python implementation into C++, including:
 - Resampling audio to 22,050 Hz
 - Framing audio into overlapping buffers (frame length: 1411 samples)
 - Computing FFT per frame
 - Extracting 272-dimensional filterbank response vectors per frame
 - Processing filterbank features (Logarithmic compression, spectral difference and concatenation)
 - Tensorize concatenated features
 - Export BeatNet (BeatNet**.model** in `stream` mode) to ONNX format
 - Apply Softmax to model predictions

# Requirements

- Python 3.9 
- Python dependencies listed in `requirements.txt`
- All C++ dependencies (onnxruntime([1.22.1](https://github.com/microsoft/onnxruntime/releases/tag/v1.22.1)), libsamplerate and  FFTW3 or KissFFT) are handled automatically.

## Configuration

| Option           | Description                               | Default |
| ---------------- | ----------------------------------------- | ------- |
| `ENABLE_KISSFFT` | Use the **Kiss FFT** library instead of FFTW3 | `OFF`   |
| `ENABLE_FFTW3`   | Use the **FFTW3** library instead of Kiss FFT | `ON`    |
| `BUILD_APP`      | Build the test application (`main.cpp`)   | `OFF`   |

> Note: `ENABLE_KISSFFT` and `ENABLE_FFTW3` cannot both be ON, while at least one FFT backend must be enabled.

**Example Usage**

Enable `Kiss FFT` and build the test app:
```
cmake -B build -D ENABLE_KISSFFT=ON -D ENABLE_FFTW3=OFF -D BUILD_APP=ON
cmake --build build
```

Enable `FFTW3` only (default) and build:
```
cmake -B build -D BUILD_APP=ON
cmake --build build
```

## Build library
```
cd BeatNet/onnx
cmake -B build <configuration options>
cmake --build build
```
This will compile the library and binaries into the build folder, with all required DLLs and model weights alongside the library (beatnetlib).

## Test inference in C++

To create a test executable, compile with `BUILD_APP` enabled:
```
cmake -B build -D BUILD_APP=ON
cmake --build build
```

Run the executable:
```
build/<Configuration>/beatnet_infer
```

The indicative console output should print info regarding the output of the model after passing random data as input to the model.

```
$ BeatNet\onnx> build\Debug\beatnet_infer.exe
...
Output shape: [1, 3, 1]
BeatNet Output: [-0.523651 -0.572624 1.00063 ]
```

# Export the Model to ONNX

To manually export the model weights:

```
cd BeatNet/onnx
python exportModel.py
```

Upon successful completion, the model weights will be exported to the current working directory as `beatnet_bda.onnx`. You can open this file in [Neutron](https://netron.app/) or a similar ONNX viewer to inspect the network’s structure, check layer shapes, and verify the model’s inputs and outputs.

# Test inference in Python
```
cd BeatNet/onnx
python testModel.py
```

