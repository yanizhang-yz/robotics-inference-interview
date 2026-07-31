# KV Cache

**Difficulty:** Medium  
**Source:** Core concept in the vLLM ecosystem — the KV cache is the centerpiece of the vLLM-anatomy write-ups and standard material in LLM-inference interview loops  
**Tags:** `kv-cache`, `attention`, `llm-serving`, `memory`, `prefill-decode`, `numpy`

## Problem

Autoregressive decoding computes attention between the newest token's query and
*every previous token's* key and value. Recomputing those K/V vectors each step is
the O(n²) mistake; caching them is how every serving engine actually decodes.
Three parts:

**A — size the cache.** `kv_cache_bytes(n_layers, n_heads, head_dim, seq_len,
batch, dtype_bytes=2) -> int` using the standard formula:

```
2 (K and V) × n_layers × n_heads × head_dim × seq_len × batch × dtype_bytes
```

**B — build one.** `class KVCache(max_seq_len)` storing one K and one V vector
(numpy arrays of shape `(d,)`) per decoded step:

- `append(k, v) -> bool` — `False` (and no change) when full, like a real cache
  hitting its sequence budget.
- `.keys` / `.values` — the cached steps stacked into `(t, d)` arrays.
- `attend(q) -> np.ndarray` — `softmax(q @ Kᵀ / sqrt(d)) @ V` over the `t` cached
  steps: exactly what one decode step's attention computes against the cache.
- `len(cache)` — number of cached steps.

**C — prove the point.** `generation_cost_with_cache(n)` vs
`generation_cost_without_cache(n)`: count per-token K/V computations for an
`n`-token generation — `n` with a cache versus `n(n+1)/2` without (exact
definitions in the docstrings), making O(n) vs O(n²) concrete and testable.

```python
kv_cache_bytes(32, 32, 128, 4096, 8)   # Llama-7B-ish, fp16
# -> 17_179_869_184 bytes = exactly 16 GiB
```

## Why this appears in robotics inference interviews

This is *the* first-principles LLM-serving question. Generation has two phases:
**prefill** (the whole prompt in one parallel pass — compute-bound, fills the
cache) and **decode** (one token at a time — each step computes K/V for just the
new token and reuses everything cached, making decode memory-bandwidth-bound).
Interviewers ask it because the real production constraint is not FLOPs but
**cache memory**: a Llama-7B-ish config (32 layers, 32 heads, head_dim 128, fp16)
at seq_len 4096 and batch 8 needs 17,179,869,184 bytes — **16 GiB for the cache
alone, before any weights** — which is why batch size, context length, and cost
per token are all really KV-cache arithmetic. The robotics tie-in: VLA robot
policies are transformers decoding action tokens, so the same prefill/decode
split and cache-size arithmetic govern on-robot inference latency.

## Approach

- **Part A** is pure arithmetic — 2 tensors per layer (K and V), each
  `n_heads × head_dim` wide per token, `seq_len × batch` tokens, `dtype_bytes`
  each. Return a plain `int`.
- **Part B**: keep two Python lists of `(d,)` arrays; `append` checks
  `len < max_seq_len` first; `keys`/`values` are `np.stack` of the lists.
  `attend` reuses the stable-softmax idea from
  `questions/03_ml_inference/002_scaled_dot_product_attention`: scores
  `K @ q / sqrt(d)`, subtract the max, exponentiate, normalize, weight-sum V.
- **Part C**, cost model (count per-token K/V computations): with a cache each of
  the `n` generated tokens has its K/V computed exactly once → `n`. Without one,
  step `t` re-runs the whole `t`-token prefix and recomputes all `t` K/V pairs →
  `1 + 2 + … + n = n(n+1)/2`.

**Time:** `append` O(1); `attend` O(t·d) per decode step. **Space:** O(t·d) —
that space *is* the cache, and Part A is its price tag.

## Follow-ups to be ready for

1. **Paged attention**: allocating one contiguous max-length K/V region per request
   wastes memory on fragmentation and unused tail. vLLM's fix mirrors OS virtual
   memory — chop the cache into fixed-size blocks (e.g. 16 tokens), let a
   per-sequence *block table* map logical positions to physical blocks, allocate
   on demand. Near-zero waste, and sequences sharing a prefix can share blocks.
2. **Continuous batching**: don't wait for the whole batch to finish — as one
   sequence completes, splice a new request into the freed cache slots at the
   next iteration.
3. **Eviction / sliding window**: cap the cache and drop the oldest entries
   (sliding-window attention) or keep the first tokens as attention sinks.
4. **MQA / GQA**: multi-query and grouped-query attention share K/V across query
   heads — divide the `n_heads` term in Part A by the sharing factor, which is
   exactly why they exist.
