"""KV cache: size formula, a working per-head cache, and the O(n) vs O(n^2)
cost model that justifies its existence.

Decode reuses every previously computed K/V vector; only the newest token's
K and V are computed per step. The price is memory — the size formula in
kv_cache_bytes is the arithmetic behind every serving engine's batch and
context-length limits.
"""

import numpy as np


def kv_cache_bytes(
    n_layers: int,
    n_heads: int,
    head_dim: int,
    seq_len: int,
    batch: int,
    dtype_bytes: int = 2,
) -> int:
    """2 (K and V) * layers * heads * head_dim * seq_len * batch * dtype_bytes."""
    return 2 * n_layers * n_heads * head_dim * seq_len * batch * dtype_bytes


class KVCache:
    """Per-step K/V store for one attention head, capped at max_seq_len steps."""

    def __init__(self, max_seq_len: int):
        self.max_seq_len = int(max_seq_len)
        self._keys: list[np.ndarray] = []
        self._values: list[np.ndarray] = []

    def __len__(self) -> int:
        return len(self._keys)

    def append(self, k: np.ndarray, v: np.ndarray) -> bool:
        """Cache one step's (d,) key and value; False (no change) when full."""
        if len(self._keys) >= self.max_seq_len:
            return False
        self._keys.append(np.asarray(k, dtype=float))
        self._values.append(np.asarray(v, dtype=float))
        return True

    @property
    def keys(self) -> np.ndarray:
        """(t, d) array of cached keys."""
        return np.stack(self._keys)

    @property
    def values(self) -> np.ndarray:
        """(t, d) array of cached values."""
        return np.stack(self._values)

    def attend(self, q: np.ndarray) -> np.ndarray:
        """softmax(q @ K^T / sqrt(d)) @ V over the cached steps (stable softmax)."""
        q = np.asarray(q, dtype=float)
        scores = self.keys @ q / np.sqrt(q.size)      # (t,)
        weights = np.exp(scores - scores.max())       # stable: subtract max
        weights /= weights.sum()
        return weights @ self.values                  # (d,)


def generation_cost_with_cache(n: int) -> int:
    """Per-token K/V computations for n generated tokens with a cache: each
    token's K/V is computed exactly once -> n."""
    return n


def generation_cost_without_cache(n: int) -> int:
    """Per-token K/V computations without a cache: step t recomputes K/V for
    its whole t-token prefix -> 1 + 2 + ... + n = n(n+1)/2."""
    return n * (n + 1) // 2
