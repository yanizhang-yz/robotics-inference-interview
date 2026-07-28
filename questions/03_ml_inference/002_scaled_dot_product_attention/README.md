# Scaled Dot-Product Attention

**Difficulty:** Medium  
**Source:** Implementing scaled dot-product attention (sometimes a full transformer block) without autograd is a commonly reported question in NVIDIA and Tesla ML interview loops (per 2025-26 interview guides)  
**Tags:** `attention`, `softmax`, `transformers`, `numerical-stability`, `kv-cache`

## Problem

Implement the core transformer op in plain numpy — no autograd, no framework:

- `softmax(x, axis=-1)` — numerically stable: subtract the max along `axis` before
  exponentiating.
- `scaled_dot_product_attention(Q, K, V, mask=None) -> (output, weights)` — compute
  `softmax(Q K^T / sqrt(d_k) + mask) V`. The optional additive `mask` holds `-inf`
  (or a very large negative number) at disallowed positions and `0` elsewhere.
- `causal_mask(n)` — the `(n, n)` additive mask for autoregressive decoding:
  query position `i` may attend key positions `j <= i` only.

Shapes: `Q (n_q, d_k)`, `K (n_k, d_k)`, `V (n_k, d_v)` → `output (n_q, d_v)`,
`weights (n_q, n_k)`. Every row of `weights` sums to 1.

```python
Q = [[2, 0, 0, 0]]              # one query, d_k = 4, so sqrt(d_k) = 2
K = [[2, 0, 0, 0],
     [0, 2, 0, 0]]              # two keys
V = [[1, 0],
     [0, 1]]                    # identity values: output IS the weight row

scores  = Q @ K.T / 2           # [[2, 0]]
weights = softmax([2, 0])       # [0.8808, 0.1192]
output  = weights @ V           # [0.8808, 0.1192]
```

## Why this appears in robotics inference interviews

Attention is the op your inference engine spends its cycles on — the interviewer is
checking that you know what the kernel you're optimizing actually computes. Three
standard probes:

- **The `sqrt(d_k)` scaling.** Dot products of `d_k`-dimensional vectors have variance
  that grows with `d_k`, so unscaled logits get large and push softmax into its
  saturated region where gradients vanish — that's the training story. The inference
  framing: scaling keeps the logits in a sane range so the weights aren't one-hot noise.
- **Mask mechanics.** The additive `-inf` mask is how causality is enforced, and it's
  the prerequisite for understanding KV-caching: at decode time each new token's query
  row sees exactly the keys the causal mask would have allowed.
- **Stable softmax.** The same subtract-max trick as the ramp-up drill: mathematically
  a no-op (the `exp(-max)` factor cancels in the ratio), numerically the difference
  between exact answers and overflow — naive `exp` overflows float64 near logits of
  709 (float32 near 88).

## Approach

1. `scores = Q @ K.T / sqrt(d_k)` with `d_k = Q.shape[-1]` — shape `(n_q, n_k)`.
2. If `mask` is given, add it. A `-inf` entry survives the subtract-max (each row's
   max stays finite as long as one position is allowed) and becomes `exp(-inf) = 0`:
   exactly zero weight, not merely small.
3. `weights = softmax(scores, axis=-1)`, then `output = weights @ V`.

`causal_mask(n)`: zeros on and below the diagonal, `-inf` strictly above
(`np.triu_indices(n, k=1)`).

**Time:** O(n_q · n_k · (d_k + d_v)) — the two matmuls.  
**Space:** O(n_q · n_k) for the weights matrix (the thing flash attention refuses to
materialize).

## Follow-ups to be ready for

1. **Multi-head shapes:** split `d_model` into `h` heads — project to `(n, d_model)`,
   reshape/transpose to `(h, n, d_model/h)`, run attention per head, concatenate back
   to `(n, d_model)`. Know the split/concat shapes cold.
2. **Why the KV-cache turns generation from O(n²) to O(n) per token:** cache K and V
   for all past tokens; each new token computes one query row against the cached keys
   instead of recomputing the full n × n score matrix from scratch.
3. **Flash attention in one line:** tile Q, K, V through on-chip SRAM and compute the
   softmax online, so the (n, n) weights matrix is never materialized in HBM.
4. **Kernel fusion:** matmul → scale → mask → softmax → matmul is a chain of
   memory-bound ops; fusing them into one kernel avoids round-trips to global memory —
   attention is the canonical fusion target.
