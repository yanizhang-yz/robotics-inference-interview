# Quantize a Tensor (INT8 Symmetric)

**Difficulty:** Medium  
**Source:** Common in inference-engineer loops at NVIDIA, Qualcomm, Apple, Waymo  
**Tags:** `quantization`, `INT8`, `model-compression`, `fixed-point`

## Problem

Implement **symmetric per-tensor INT8 quantization** and its inverse (dequantization).

Given a float32 tensor, compute:
- `scale = max(|tensor|) / 127`
- `quantized = round(clip(tensor / scale, -128, 127)).astype(int8)`

Then implement dequantization:
- `dequantized = quantized * scale`

```python
tensor = [0.0, 1.0, -1.0, 0.5, -0.25]
# scale = 1.0 / 127 ≈ 0.00787
# quantized ≈ [0, 127, -127, 64, -32]
```

## Why this appears in robotics inference interviews

Quantization is the core technique for deploying neural networks on embedded hardware (Jetson, TPU, MCUs). Inference engineers must understand the math, the precision loss, and calibration strategies. Expect follow-ups on:

- Per-channel vs per-tensor quantization
- Asymmetric (zero-point) quantization
- Quantization-aware training (QAT)
- Why INT4 is hard for activations

## Approach

Symmetric quantization maps `[-max_val, +max_val]` → `[-127, 127]` with a single scale factor and no zero-point. Simple to implement in hardware; loses about half the INT8 range for always-positive activations (use asymmetric there).
