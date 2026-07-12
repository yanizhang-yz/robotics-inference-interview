"""
NumPy essentials — reference solutions.

The recurring lesson: replace Java's element-by-element loops with
whole-array operations, boolean masks, axis reductions, and broadcasting.
No function in this file contains a loop.
"""

import numpy as np


def normalize(v: np.ndarray) -> np.ndarray:
    norm = np.linalg.norm(v)
    if norm == 0:
        # Same shape, float dtype, all zeros — avoids 0/0 -> NaN.
        return np.zeros_like(v, dtype=float)
    return v / norm


def relu(x: np.ndarray) -> np.ndarray:
    # Elementwise max against a scalar; the 0 broadcasts to x's shape.
    return np.maximum(x, 0)


def softmax(logits: np.ndarray) -> np.ndarray:
    # Subtracting the max shifts every exponent into (-inf, 0], so exp()
    # cannot overflow. Softmax is invariant to the shift — the e^{-max}
    # factors cancel between numerator and denominator.
    exps = np.exp(logits - logits.max())
    return exps / exps.sum()


def one_hot(labels: np.ndarray, num_classes: int) -> np.ndarray:
    # Fancy indexing: indexing the identity matrix with an integer array
    # picks whole rows — row labels[i] of eye(k) IS the one-hot for labels[i].
    return np.eye(num_classes)[labels]


def count_above(x: np.ndarray, threshold: float) -> int:
    # (x > threshold) is a boolean ARRAY; summing counts the Trues.
    # int(...) converts numpy's scalar type to a plain Python int.
    return int((x > threshold).sum())


def clamp_negatives(x: np.ndarray) -> np.ndarray:
    # .copy() first — mask assignment mutates in place, and without the
    # copy we'd clobber the caller's array (a classic numpy bug).
    out = x.copy()
    out[out < 0] = 0
    return out


def row_argmax(mat: np.ndarray) -> np.ndarray:
    # axis=1 collapses the column dimension: one argmax per row,
    # result shape (n_rows,).
    return mat.argmax(axis=1)


def moving_average(x: np.ndarray, k: int) -> np.ndarray:
    # Convolving with a length-k box of ones gives window sums; "valid"
    # keeps only fully-overlapping windows (len(x) - k + 1 of them).
    return np.convolve(x, np.ones(k), mode="valid") / k


def pairwise_distances(a: np.ndarray, b: np.ndarray) -> np.ndarray:
    # Broadcasting: (n, 1, d) - (1, m, d) -> (n, m, d). Each size-1 axis
    # stretches to match, giving every a[i] - b[j] pair without loops.
    diff = a[:, None, :] - b[None, :, :]
    return np.sqrt((diff**2).sum(axis=-1))


def standardize_columns(mat: np.ndarray) -> np.ndarray:
    # axis=0 collapses rows: mean/std are per-column, shape (n_cols,),
    # and broadcast back over the rows on subtraction/division.
    mean = mat.mean(axis=0)
    std = mat.std(axis=0)
    # Zero-std (constant) columns: divide by 1 instead of 0. The numerator
    # is already all zeros there, so the column standardizes to zeros.
    safe_std = np.where(std == 0, 1.0, std)
    return (mat - mean) / safe_std
