"""Scaled dot-product attention in plain numpy.

softmax subtracts the row max before exponentiating: mathematically a no-op
(the exp(-max) factor cancels in the ratio), numerically the difference
between exact answers and overflow — naive exp overflows float64 near 709.
Masks are additive: -inf at a disallowed position becomes exp(-inf) = 0,
i.e. exactly zero weight, and the subtract-max keeps masked rows finite as
long as at least one position is allowed.
"""

import numpy as np


def softmax(x: np.ndarray, axis: int = -1) -> np.ndarray:
    """Numerically stable softmax along `axis`."""
    x = np.asarray(x, dtype=float)
    shifted = x - x.max(axis=axis, keepdims=True)
    e = np.exp(shifted)
    return e / e.sum(axis=axis, keepdims=True)


def scaled_dot_product_attention(
    Q: np.ndarray, K: np.ndarray, V: np.ndarray, mask: np.ndarray | None = None
) -> tuple[np.ndarray, np.ndarray]:
    """softmax(Q K^T / sqrt(d_k) + mask) V.

    Shapes: Q (n_q, d_k), K (n_k, d_k), V (n_k, d_v)
            -> output (n_q, d_v), weights (n_q, n_k).
    """
    Q = np.asarray(Q, dtype=float)
    K = np.asarray(K, dtype=float)
    V = np.asarray(V, dtype=float)

    d_k = Q.shape[-1]
    scores = Q @ K.T / np.sqrt(d_k)
    if mask is not None:
        scores = scores + mask
    weights = softmax(scores, axis=-1)
    return weights @ V, weights


def causal_mask(n: int) -> np.ndarray:
    """Additive (n, n) mask: 0.0 where j <= i, -inf where j > i."""
    mask = np.zeros((n, n))
    mask[np.triu_indices(n, k=1)] = -np.inf
    return mask
