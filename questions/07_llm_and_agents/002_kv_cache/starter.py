"""
KV Cache — YOUR ATTEMPT

Write your solution here, then run the tests against it:
    PRACTICE=1 uv run pytest questions/07_llm_and_agents/002_kv_cache -v

Peek at solution.py only after you've tried.
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
    """
    Bytes needed to cache K and V for a decoder-only transformer:

        2 (K and V) * n_layers * n_heads * head_dim * seq_len * batch * dtype_bytes

    Return a plain int. Sanity anchor: (32, 32, 128, 4096, 8) in fp16 is 16 GiB.
    """
    # TODO: implement
    raise NotImplementedError


class KVCache:
    """Per-step K/V store for one attention head: one (d,) key and one (d,)
    value vector per decoded token, capped at max_seq_len steps."""

    def __init__(self, max_seq_len: int):
        # TODO: store the cap and two empty lists (keys, values)
        raise NotImplementedError

    def __len__(self) -> int:
        """Number of cached steps t."""
        # TODO: implement
        raise NotImplementedError

    def append(self, k: np.ndarray, v: np.ndarray) -> bool:
        """
        Cache one step's key and value (each shape (d,)).
        Return False and change NOTHING when already at max_seq_len.
        """
        # TODO: implement
        raise NotImplementedError

    @property
    def keys(self) -> np.ndarray:
        """All cached keys stacked into a (t, d) array (np.stack)."""
        # TODO: implement
        raise NotImplementedError

    @property
    def values(self) -> np.ndarray:
        """All cached values stacked into a (t, d) array."""
        # TODO: implement
        raise NotImplementedError

    def attend(self, q: np.ndarray) -> np.ndarray:
        """
        One decode step's attention against the cache:
            softmax(q @ K^T / sqrt(d)) @ V        over the t cached steps.

        Plan:
        1. scores = keys @ q / sqrt(d)                    # shape (t,)
        2. Stable softmax: w = exp(scores - scores.max()); w /= w.sum()
        3. Return w @ values                              # shape (d,)
        """
        # TODO: implement
        raise NotImplementedError


def generation_cost_with_cache(n: int) -> int:
    """
    Per-token K/V computations to generate n tokens WITH a cache.

    Each new token's K and V are computed once, cached, and reused by every
    later step, so the total is exactly n.
    """
    # TODO: implement
    raise NotImplementedError


def generation_cost_without_cache(n: int) -> int:
    """
    Per-token K/V computations to generate n tokens WITHOUT a cache.

    Step t re-runs the whole t-token prefix and recomputes K/V for all t
    tokens, so the total is 1 + 2 + ... + n = n * (n + 1) // 2.
    """
    # TODO: implement
    raise NotImplementedError
