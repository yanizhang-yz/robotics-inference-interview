# 2D Convolution from Scratch

**Difficulty:** Medium  
**Source:** Implementing conv2d's forward pass from scratch is a commonly reported question in NVIDIA and Tesla ML interview loops (per 2025-26 interview guides)  
**Tags:** `convolution`, `cnn`, `numpy`, `stride`, `padding`

## Problem

Implement the forward pass of 2D convolution for a **single-channel** image in plain numpy — no deep-learning libraries.

```python
conv2d(image, kernel, stride=1, padding=0) -> np.ndarray
output_shape(h, w, kh, kw, stride=1, padding=0) -> (oh, ow)
```

`output_shape` implements the standard formula, applied to both dimensions:

```
oh = (h + 2*padding - kh) // stride + 1
```

`conv2d` zero-pads the image, slides the kernel, and returns the output feature map. Raise `ValueError` if the kernel is larger than the padded image.

```
Input:
  image  = [[ 1,  2,  3,  4],
            [ 5,  6,  7,  8],
            [ 9, 10, 11, 12],
            [13, 14, 15, 16]]
  kernel = 3x3 box blur (all entries 1/9), stride=1, padding=0

Output:
  [[ 6.,  7.],
   [10., 11.]]
```

## Why this appears in robotics inference interviews

Convolution is the workhorse op of every vision backbone you deploy — ResNet feature extractors, YOLO detection heads, segmentation encoders. Interviewers use this to check that you know what the layers you deploy actually compute, including how padding and stride determine output size — the arithmetic you use daily when reading model architectures or debugging a shape mismatch in an inference pipeline.

## Approach

1. Zero-pad the image with `np.pad(image, padding)`.
2. Compute `(oh, ow)` from the output-shape formula and allocate the output.
3. For each output cell `(i, j)`, take the `kh x kw` patch starting at `(i*stride, j*stride)` and set `out[i, j] = np.sum(patch * kernel)`.

Note that this is **cross-correlation**: the kernel is slid over the image without being flipped. True (signal-processing) convolution flips the kernel 180 degrees first; deep-learning frameworks skip the flip because a learned kernel makes it redundant, and they call the unflipped op "convolution" anyway. Say this out loud in an interview — it signals you know the difference.

Explicit loops over output pixels are acceptable and expected here. Production frameworks lower convolution to **im2col + one big matmul** so it runs on BLAS/tensor cores — a standard follow-up (see below).

**Time:** O(oh · ow · kh · kw).  
**Space:** O(oh · ow) for the output, plus the padded copy.

## Follow-ups

- **Multi-channel, multi-filter:** real conv layers map `(C_in, H, W)` inputs through `(C_out, C_in, kh, kw)` weights — each output channel sums a per-input-channel 2D conv over all `C_in` channels. Work out the output shape.
- **im2col:** unroll every patch into a row of a matrix, so the whole conv becomes one matmul with the flattened kernel — this is how frameworks make it fast. What does it cost in memory?
- **Separable kernels:** Sobel-x is the outer product of `[1, 2, 1]` and `[-1, 0, 1]`, so it can run as two 1-D passes — per-pixel cost drops from O(kh·kw) to O(kh+kw).
- **Dilation:** space the kernel taps `d` pixels apart (effective kernel size `d*(k-1)+1`). How does the output-shape formula change?
