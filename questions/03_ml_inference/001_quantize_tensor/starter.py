"""
Quantize a Tensor (INT8 Symmetric) — YOUR ATTEMPT

Write your solution here, then run the tests against it:
    PRACTICE=1 uv run pytest questions/03_ml_inference/001_quantize_tensor -v

Peek at solution.py only after you've tried.
"""

import numpy as np


def quantize_symmetric_int8(tensor: np.ndarray) -> tuple[np.ndarray, float]:
    """
    Symmetric per-tensor INT8 quantization. Return (quantized_tensor, scale).

    Hint: scale = max(|tensor|) / 127; quantized = round(tensor / scale),
    clipped to [-128, 127], cast to int8. Handle the all-zeros tensor.
    """
    # TODO: implement
    raise NotImplementedError


def dequantize(quantized: np.ndarray, scale: float) -> np.ndarray:
    """Reconstruct float32 values from the INT8 tensor and scale."""
    # TODO: implement
    raise NotImplementedError
