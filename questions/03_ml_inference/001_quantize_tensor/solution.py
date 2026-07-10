import numpy as np


def quantize_symmetric_int8(tensor: np.ndarray) -> tuple[np.ndarray, float]:
    """
    Symmetric per-tensor INT8 quantization.
    Returns (quantized_tensor, scale).
    """
    max_val = np.max(np.abs(tensor))
    if max_val == 0:
        return np.zeros_like(tensor, dtype=np.int8), 1.0

    scale = max_val / 127.0
    quantized = np.clip(np.round(tensor / scale), -128, 127).astype(np.int8)
    return quantized, scale


def dequantize(quantized: np.ndarray, scale: float) -> np.ndarray:
    """Reconstructs float32 values from INT8 tensor and scale."""
    return quantized.astype(np.float32) * scale
