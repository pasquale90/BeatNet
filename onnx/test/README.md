This directory is used to test the porting of the BeatNet model into C++.
The following configuration is used:

- Samples are loaded from the `samples` directory.
- Sample rate: 22050 Hz
- Buffer size: 512
- BeatNet operates in `Streaming mode`.

Two implementations are compared:
1. The official Python implementation.
2. The C++ ported implementation.

The goal is to verify the correctness of the C++ implementation by comparing its results with the Python implementation. The comparison process involves the following steps:

1. Load an audio file and divide it into buffers.
2. Feed each buffer into the model for predictions.
3. Annotate each buffer with the model's predictions.
4. Store the results in files.
5. Compare the alignment of predictions between the two implementations.

# Results

Results are yet to be defined.

# Help

- [condaEnvList](condaEnvList) file is provided for helping out in the python env setup.