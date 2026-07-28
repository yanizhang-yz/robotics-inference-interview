"""
Token Sampling: Temperature, Top-k, Top-p — YOUR ATTEMPT

Write your solution here, then run the tests against it:
    PRACTICE=1 uv run pytest questions/07_llm_and_agents/001_sampling_topk_topp -v

Peek at solution.py only after you've tried.
"""

import numpy as np


def softmax(logits: np.ndarray) -> np.ndarray:
    """Stable softmax — given, not part of the exercise. -inf entries get 0 mass."""
    logits = np.asarray(logits, dtype=float)
    shifted = np.exp(logits - np.max(logits))
    return shifted / shifted.sum()


def greedy_sample(logits: np.ndarray) -> int:
    """Return the argmax token id as a plain int."""
    # TODO: implement
    raise NotImplementedError


def apply_temperature(logits: np.ndarray, temperature: float) -> np.ndarray:
    """
    Return logits / temperature as a float array.

    Raise ValueError if temperature <= 0. Remember what the knob does:
    t -> 0 sharpens the eventual softmax toward greedy (gaps between logits
    grow), t > 1 flattens it toward uniform (gaps shrink).
    """
    # TODO: implement
    raise NotImplementedError


def top_k_filter(logits: np.ndarray, k: int) -> np.ndarray:
    """
    Keep the k highest logits; set every other entry to -np.inf.

    Plan:
    1. Raise ValueError if k < 1. If k >= len(logits), nothing to filter.
    2. Indices of the k largest: np.argsort(-logits, kind="stable")[:k].
    3. Build a -inf array, copy the kept entries in. Do not mutate the input.
    """
    # TODO: implement
    raise NotImplementedError


def top_p_filter(probs: np.ndarray, p: float) -> np.ndarray:
    """
    Nucleus filtering on a probability vector.

    Plan:
    1. Raise ValueError unless 0 < p <= 1.
    2. Sort descending (argsort of -probs), take the cumulative sum.
    3. Keep the smallest prefix with cumsum >= p — np.searchsorted(cumsum, p)
       gives the cutoff; clamp it to the last index so at least one token
       (and at most all of them) survives. If the top token alone has
       probability >= p, the nucleus is just that token.
    4. Zero everything outside the nucleus, then RENORMALIZE so the kept
       entries sum to 1. Forgetting the renormalize is the classic trap.
    """
    # TODO: implement
    raise NotImplementedError


def sample(
    logits: np.ndarray,
    temperature: float = 1.0,
    top_k: int | None = None,
    top_p: float | None = None,
    rng: np.random.Generator | None = None,
) -> int:
    """
    Full serving-engine sampling pipeline. Returns a token id.

    Plan (order matters):
    1. rng = np.random.default_rng(0) when rng is None (deterministic default).
    2. logits = apply_temperature(logits, temperature)   # validates t > 0
    3. If top_k is not None: logits = top_k_filter(logits, top_k)
    4. probs = softmax(logits)
    5. If top_p is not None: probs = top_p_filter(probs, top_p)
    6. Return int(rng.choice(len(probs), p=probs)).
    """
    # TODO: implement
    raise NotImplementedError
