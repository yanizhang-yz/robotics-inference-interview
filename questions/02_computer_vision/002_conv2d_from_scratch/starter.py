"""
2D Convolution from Scratch — YOUR ATTEMPT

Write your solution here, then run the tests against it:
    PRACTICE=1 uv run pytest questions/02_computer_vision/002_conv2d_from_scratch -v

Peek at solution.py only after you've tried.
"""

import numpy as np


def output_shape(
    h: int, w: int, kh: int, kw: int, stride: int = 1, padding: int = 0
) -> tuple[int, int]:
    """
    Output (height, width) of a conv layer, via the standard formula
    applied to each dimension:

        oh = (h + 2*padding - kh) // stride + 1
    """
    # TODO: implement
    raise NotImplementedError


def conv2d(
    image: np.ndarray, kernel: np.ndarray, stride: int = 1, padding: int = 0
) -> np.ndarray:
    """
    Forward pass of single-channel 2D convolution (cross-correlation).

    Plan:
    1. np.asarray both inputs as float; record (h, w) and (kh, kw).
    2. Raise ValueError if the kernel is larger than the PADDED image
       (kh > h + 2*padding or kw > w + 2*padding).
    3. Zero-pad: np.pad(image, padding).
    4. oh, ow = output_shape(...); allocate np.zeros((oh, ow)).
    5. For each output cell (i, j):
       patch = padded[i*stride : i*stride + kh, j*stride : j*stride + kw]
       out[i, j] = np.sum(patch * kernel)
       (No kernel flip — this is cross-correlation, which is what
       deep-learning frameworks call "convolution".)
    """
    # TODO: implement
    raise NotImplementedError
