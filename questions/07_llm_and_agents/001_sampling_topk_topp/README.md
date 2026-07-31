# Token Sampling: Temperature, Top-k, Top-p

**Difficulty:** Medium  
**Source:** Core concepts in the vLLM ecosystem — sampling configuration is standard material in LLM-inference interview loops (the vLLM-anatomy write-ups walk through exactly these knobs)  
**Tags:** `sampling`, `softmax`, `temperature`, `top-k`, `top-p`, `llm-serving`, `numpy`

## Problem

A language model's forward pass ends with a vector of **logits** — one raw score per
vocabulary token. Turning logits into the *next token id* is the sampling step, and
every serving engine exposes the same three knobs. Implement them in plain numpy:

- `greedy_sample(logits) -> int` — the argmax token id.
- `apply_temperature(logits, temperature) -> logits` — divide logits by `temperature`
  (must be > 0, else `ValueError`). As `t → 0` the softmax sharpens toward greedy;
  `t > 1` flattens it toward uniform.
- `top_k_filter(logits, k) -> logits` — keep the `k` highest logits, set every other
  entry to `-inf` (`ValueError` if `k < 1`).
- `top_p_filter(probs, p) -> probs` — **nucleus** filtering. Operates on
  *probabilities*: sort descending, keep the smallest prefix whose cumulative
  probability is `>= p`, zero out the rest, **renormalize** to sum to 1
  (`ValueError` unless `0 < p <= 1`).
- `sample(logits, temperature=1.0, top_k=None, top_p=None, rng=None) -> int` —
  compose them in serving-engine order: temperature → top-k → softmax → top-p →
  draw with `rng.choice`. `rng=None` means `np.random.default_rng(0)` so the
  default is deterministic.

```python
logits = [2.0, 1.0, 0.1]
greedy_sample(logits)                        # -> 0
top_k_filter(logits, k=2)                    # -> [2.0, 1.0, -inf]
top_p_filter([0.5, 0.25, 0.125, 0.125], 0.7) # cumsum [0.5, 0.75, ...] — first
                                             # prefix >= 0.7 is two tokens
                                             # -> [2/3, 1/3, 0.0, 0.0]
```

## Why this appears in robotics inference interviews

The sampling config is the **user-facing knob of every serving engine** — every
request that hits vLLM, TGI, or an in-house server carries a temperature/top-k/top-p
triple, so interviewers check that you know what those parameters mathematically
*do*, not just that they exist. The classic trap they probe: top-p operates on
**sorted cumulative probabilities** and must **renormalize** afterward — candidates
who filter without re-sorting, or return a "distribution" that no longer sums to 1,
fail the follow-up. The robotics tie-in is direct: modern robot policies are VLA
(vision-language-action) transformers that emit action tokens through this exact
pipeline — they mostly run it in greedy mode, which is one of the follow-ups below.

## Approach

Pipeline order is the whole design: **logits-space ops first** (temperature scales,
top-k truncates with `-inf`, which softmax maps to probability 0), then softmax,
then **probability-space ops** (top-p), then the draw.

1. Temperature: `logits / t`. Dividing by a small `t` stretches gaps between logits
   (softmax → near one-hot); a large `t` shrinks them (softmax → near uniform).
2. Top-k: find the `k` largest entries (`argsort` descending, keep the first `k`),
   set all others to `-inf`. After softmax those tokens have exactly zero mass.
3. Softmax: stable form — subtract the max before exponentiating (the `-inf`
   entries become `exp(-inf) = 0`).
4. Top-p: sort probabilities descending, take the cumulative sum, keep the
   smallest prefix with `cumsum >= p` (at least one token — if the top token alone
   exceeds `p`, the nucleus is just that token), zero the rest, renormalize.
5. Draw: `rng.choice(vocab_size, p=probs)`.

**Time:** O(V log V) for the sorts (top-k can be O(V) with `argpartition`).
**Space:** O(V).

## Follow-ups to be ready for

1. **Repetition penalty**: divide (or subtract from) the logits of already-generated
   tokens before sampling — a logits-space op, so it slots in before temperature.
2. **Min-p**: keep tokens whose probability is at least `min_p * max(probs)` — an
   alternative to top-p that adapts to how peaked the distribution is.
3. **Beam search vs sampling**: beam search keeps the `B` highest-probability
   *sequences* (deterministic, favors safe completions); sampling explores. Chat
   wants sampling; translation-style tasks historically used beams.
4. **Why robot VLA policies mostly decode greedily**: a robot arm wants the same
   action for the same observation — determinism and repeatability beat diversity,
   so temperature ~0 / argmax is the norm.
5. **Speculative decoding** in one line: a small draft model proposes several
   tokens, the big model verifies them in a single batched forward pass — same
   distribution, fewer big-model steps.
