"""Forward pass of 2D convolution in plain numpy.

What deep-learning frameworks call "convolution" is cross-correlation:
the kernel slides over the image WITHOUT the 180-degree flip that true
(signal-processing) convolution applies. Networks learn their kernels,
so the flip is redundant and frameworks skip it; this implementation
does the same.

Explicit loops over output pixels are fine at interview scale —
frameworks get their speed by lowering conv to im2col + one big matmul
(see the README follow-ups).
"""

import numpy as np


def output_shape(
    h: int, w: int, kh: int, kw: int, stride: int = 1, padding: int = 0
) -> tuple[int, int]:
    """Output (height, width): (dim + 2*padding - k) // stride + 1 per axis."""
    oh = (h + 2 * padding - kh) // stride + 1
    ow = (w + 2 * padding - kw) // stride + 1
    return oh, ow


def conv2d(
    image: np.ndarray, kernel: np.ndarray, stride: int = 1, padding: int = 0
) -> np.ndarray:
    """Single-channel 2D convolution (cross-correlation) forward pass."""
    image = np.asarray(image, dtype=float)
    kernel = np.asarray(kernel, dtype=float)
    h, w = image.shape
    kh, kw = kernel.shape

    if kh > h + 2 * padding or kw > w + 2 * padding:
        raise ValueError(
            f"kernel {kh}x{kw} is larger than the padded image "
            f"{h + 2 * padding}x{w + 2 * padding}"
        )

    if padding > 0:
        image = np.pad(image, padding)  # zero-pad both axes

    oh, ow = output_shape(h, w, kh, kw, stride, padding)
    out = np.zeros((oh, ow))
    for i in range(oh):
        for j in range(ow):
            r, c = i * stride, j * stride
            out[i, j] = np.sum(image[r : r + kh, c : c + kw] * kernel)
    return out
