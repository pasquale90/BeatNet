import sys
import os
sys.path.insert(0, os.path.abspath("../src"))
import torch
from BeatNet.BeatNet import BeatNet

# Initialize BeatNet
estimator = BeatNet(1, mode='offline', inference_model='DBN', plot=[], thread=False)

# Get the PyTorch model (BDA)
model = estimator.model

# set to eval mode
model.eval()

# move it to the CPU
device = torch.device("cpu")
model.to(device)

# Create dummy input (batch_size, time_steps, feature_dim)
print("Expected dim_in:", model.dim_in)
dummy_input = torch.randn(1, 1, 272).to(device)

# export to ONNX
torch.onnx.export(
    model,
    dummy_input,
    "beatnet_bda.onnx",
    input_names=["input"],
    output_names=["output"],
    dynamic_axes={"input": {0: "batch", 1: "time"}, "output": {0: "batch", 2: "time"}},
    opset_version=17
)

print("Exported to beatnet_bda.onnx")