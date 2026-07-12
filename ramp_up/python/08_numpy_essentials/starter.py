"""
NumPy essentials — YOUR ATTEMPT

Write your code here, then run the tests against it:
    PRACTICE=1 uv run pytest ramp_up/python/08_numpy_essentials -v

Peek at solution.py only after you've tried. Rule of the module: if you are
writing `for` over array elements, there is a whole-array operation you're
missing. None of these functions needs a loop.
"""

import numpy as np


def normalize(v: np.ndarray) -> np.ndarray:
    """
    Return v scaled to unit length. For the zero vector, return a
    same-shape array of zeros (no division by zero, no NaN).

    JAVA: one loop for the sum of squares, sqrt, then a second loop dividing.
    PYTHON: v / np.linalg.norm(v) — whole-array division; guard norm == 0.
    """
    raise NotImplementedError


def relu(x: np.ndarray) -> np.ndarray:
    """
    Elementwise max(x, 0). Works on any shape. NO loops.

    JAVA: for (int i...) out[i] = Math.max(x[i], 0);
    PYTHON: np.maximum(x, 0) — elementwise max against a broadcast scalar.
    """
    raise NotImplementedError


def softmax(logits: np.ndarray) -> np.ndarray:
    """
    Softmax over a 1-D array of logits: exp(x_i) / sum(exp(x)).
    Must be numerically stable for large logits (e.g. 1000) — subtract
    the max first; softmax(x) == softmax(x - c). See README.

    JAVA: loop computing exps, loop summing, loop dividing — and it still
        overflows unless you remembered the max-subtraction trick.
    PYTHON: e = np.exp(logits - logits.max()); return e / e.sum().
    """
    raise NotImplementedError


def one_hot(labels: np.ndarray, num_classes: int) -> np.ndarray:
    """
    Return an array of shape (len(labels), num_classes) of 0.0/1.0 where
    row i has a 1.0 at column labels[i].

    JAVA: new double[n][k] + a loop setting out[i][labels[i]] = 1.
    PYTHON: np.eye(num_classes)[labels] — fancy indexing picks whole rows.
    """
    raise NotImplementedError


def count_above(x: np.ndarray, threshold: float) -> int:
    """
    Count elements strictly greater than threshold. Return a plain int.

    JAVA: counter loop: if (x[i] > t) count++.
    PYTHON: (x > threshold).sum() — a boolean mask sums as 0s and 1s.
    """
    raise NotImplementedError


def clamp_negatives(x: np.ndarray) -> np.ndarray:
    """
    Return a COPY of x with negative entries set to 0. The input array
    must NOT be mutated (tests check this).

    JAVA: defensive-copy loop with Math.max — mutation bugs are rarer
        because arrays are copied explicitly.
    PYTHON: out = x.copy(); out[out < 0] = 0 — boolean-mask assignment.
        Without the .copy() you'd silently clobber the caller's array.
    """
    raise NotImplementedError


def row_argmax(mat: np.ndarray) -> np.ndarray:
    """
    For a 2-D matrix, return a 1-D array with the column index of the
    max of each ROW (shape (n_rows,)).

    JAVA: per-row loop tracking bestIdx and bestVal.
    PYTHON: mat.argmax(axis=1) — axis=1 collapses the column dimension,
        leaving one answer per row.
    """
    raise NotImplementedError


def moving_average(x: np.ndarray, k: int) -> np.ndarray:
    """
    Moving average with window k, "valid" mode only: output has
    len(x) - k + 1 entries, each the mean of a full window.

    JAVA: sliding-window loop maintaining a running sum.
    PYTHON: np.convolve(x, np.ones(k), mode="valid") / k (or a cumsum trick).
    """
    raise NotImplementedError


def pairwise_distances(a: np.ndarray, b: np.ndarray) -> np.ndarray:
    """
    Given a of shape (n, d) and b of shape (m, d), return an (n, m) matrix
    where entry [i, j] is the Euclidean distance between a[i] and b[j].
    NO loops — this is THE broadcasting drill.

    JAVA: triple nested loop over n, m, and d.
    PYTHON: diff = a[:, None, :] - b[None, :, :]  # (n, m, d) via broadcasting
        then sqrt of the squared diff summed over the last axis.
    """
    raise NotImplementedError


def standardize_columns(mat: np.ndarray) -> np.ndarray:
    """
    Standardize each COLUMN of a 2-D matrix to mean 0 and std 1.
    Columns with zero std (constant columns) must come out as all zeros,
    not NaN. Do not mutate the input.

    JAVA: two passes of column-major loops (mean, then std), plus an
        if (std == 0) branch inside the divide loop.
    PYTHON: (mat - mat.mean(axis=0)) / np.where(std == 0, 1, std) —
        axis=0 collapses rows, giving per-column stats that broadcast back.
    """
    raise NotImplementedError
