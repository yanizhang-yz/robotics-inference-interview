"""Token sampling: temperature, top-k, top-p (nucleus).

Pipeline order is the design: logits-space ops first (temperature scales,
top-k truncates with -inf, which softmax maps to exactly zero mass), then
softmax, then probability-space ops (top-p, which must renormalize), then
the draw. Filters never mutate their input.
"""

import numpy as np


def softmax(logits: np.ndarray) -> np.ndarray:
    """Stable softmax: subtract the max first. -inf entries get 0 mass."""
    logits = np.asarray(logits, dtype=float)
    shifted = np.exp(logits - np.max(logits))
    return shifted / shifted.sum()


def greedy_sample(logits: np.ndarray) -> int:
    """Argmax token id."""
    return int(np.argmax(np.asarray(logits)))


def apply_temperature(logits: np.ndarray, temperature: float) -> np.ndarray:
    """logits / temperature. t -> 0 sharpens toward greedy; t > 1 flattens."""
    if temperature <= 0:
        raise ValueError(f"temperature must be > 0, got {temperature}")
    return np.asarray(logits, dtype=float) / temperature


def top_k_filter(logits: np.ndarray, k: int) -> np.ndarray:
    """Keep the k highest logits; every other entry becomes -inf."""
    if k < 1:
        raise ValueError(f"top_k must be >= 1, got {k}")
    logits = np.asarray(logits, dtype=float)
    if k >= logits.size:
        return logits.copy()
    keep = np.argsort(-logits, kind="stable")[:k]
    out = np.full_like(logits, -np.inf)
    out[keep] = logits[keep]
    return out


def top_p_filter(probs: np.ndarray, p: float) -> np.ndarray:
    """Nucleus filtering: smallest descending-sorted prefix with cumsum >= p,
    renormalized. If the top token alone reaches p, the nucleus is just it."""
    if not 0.0 < p <= 1.0:
        raise ValueError(f"top_p must satisfy 0 < p <= 1, got {p}")
    probs = np.asarray(probs, dtype=float)
    order = np.argsort(-probs, kind="stable")
    cumsum = np.cumsum(probs[order])
    # First index where cumsum >= p; clamp so float drift at p=1.0 (cumsum
    # ending at 0.999...) still keeps at least the full set's last token.
    cutoff = min(int(np.searchsorted(cumsum, p, side="left")), probs.size - 1)
    keep = order[: cutoff + 1]
    out = np.zeros_like(probs)
    out[keep] = probs[keep]
    return out / out.sum()


def sample(
    logits: np.ndarray,
    temperature: float = 1.0,
    top_k: int | None = None,
    top_p: float | None = None,
    rng: np.random.Generator | None = None,
) -> int:
    """Temperature -> top-k -> softmax -> top-p -> draw. Deterministic default rng."""
    if rng is None:
        rng = np.random.default_rng(0)
    logits = apply_temperature(logits, temperature)
    if top_k is not None:
        logits = top_k_filter(logits, top_k)
    probs = softmax(logits)
    if top_p is not None:
        probs = top_p_filter(probs, top_p)
    return int(rng.choice(probs.size, p=probs))
