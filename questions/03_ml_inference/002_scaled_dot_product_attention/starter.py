"""
Scaled Dot-Product Attention — YOUR ATTEMPT

Write your solution here, then run the tests against it:
    PRACTICE=1 uv run pytest questions/03_ml_inference/002_scaled_dot_product_attention -v

Peek at solution.py only after you've tried.
"""

import numpy as np


def softmax(x: np.ndarray, axis: int = -1) -> np.ndarray:
    """
    Numerically stable softmax along `axis`.

    Plan:
    1. m = x.max(axis=axis, keepdims=True). Subtracting it changes nothing
       mathematically — exp(x - m) / sum exp(x - m) cancels the exp(-m)
       factor — but it makes every exponent <= 0, so exp never overflows.
    2. e = np.exp(x - m); return e / e.sum(axis=axis, keepdims=True).
    """
    # TODO: implement
    raise NotImplementedError


def scaled_dot_product_attention(
    Q: np.ndarray, K: np.ndarray, V: np.ndarray, mask: np.ndarray | None = None
) -> tuple[np.ndarray, np.ndarray]:
    """
    Return (output, weights) = (softmax(Q K^T / sqrt(d_k) + mask) V, the weights).

    Shapes: Q (n_q, d_k), K (n_k, d_k), V (n_k, d_v)
            -> output (n_q, d_v), weights (n_q, n_k); weight rows sum to 1.

    Plan:
    1. scores = Q @ K.T / np.sqrt(d_k)   with d_k = Q.shape[-1]
    2. if mask is not None: scores = scores + mask. A -inf entry becomes
       exp(-inf) = 0 after the subtract-max — exactly zero weight — and each
       row's max stays finite as long as one position is allowed.
    3. weights = softmax(scores, axis=-1); output = weights @ V.
    """
    # TODO: implement
    raise NotImplementedError


def causal_mask(n: int) -> np.ndarray:
    """
    (n, n) additive mask for autoregressive decoding: 0.0 where query i may
    attend key j (j <= i), -inf where it may not (j > i).

    Plan: start from np.zeros((n, n)); set the strict upper triangle to
    -np.inf using np.triu_indices(n, k=1).
    """
    # TODO: implement
    raise NotImplementedError
