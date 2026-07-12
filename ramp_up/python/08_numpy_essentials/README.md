# 08 — NumPy essentials

## What this covers

The core numpy vocabulary an inference interview assumes: vectorized ops
instead of loops, boolean masks, the `axis` mental model, fancy indexing,
and broadcasting. These ten drills cover most of the numpy you'll actually
write when implementing softmax, normalization, distance matrices, and
pre/post-processing by hand.

## Why this trips up Java developers

In Java, "do X to every element" means a `for` loop over `double[]` — that's
the *only* idiom, and it's fine. In numpy, a loop over elements is a bug in
disguise: each element access goes through the interpreter, so a loop is
often 100x slower than the vectorized call that does the same work in C.
The mental shift: **stop thinking "for each element", start thinking "which
whole-array operation expresses this?"**

Three ideas carry the whole module:

1. **Vectorization + masks.** `x > 0` doesn't return a boolean — it returns
   a boolean *array*. That array can be summed (`True` counts as 1), used to
   index (`x[mask]`), or assigned through (`x[mask] = 0`). This replaces
   almost every filtering/counting loop you'd write in Java.
2. **The `axis` mental model.** `axis=N` means "collapse dimension N".
   For a `(rows, cols)` matrix: `axis=0` collapses rows, producing one value
   per *column*; `axis=1` collapses columns, producing one value per *row*.
   When unsure, check the result shape: the axis you pass is the one that
   disappears.
3. **Broadcasting.** Arrays with different shapes can combine: size-1
   dimensions stretch to match. Inserting a new axis with `None` (alias for
   `np.newaxis`) sets this up deliberately — `a[:, None, :] - b[None, :, :]`
   turns an (n,d) and an (m,d) array into an (n,m,d) difference cube with
   zero Java-style nested loops.

Two habits that bite people coming from Java:

- **Views vs copies.** Slices are *views*; `out = x; out[mask] = 0` mutates
  the caller's array. Use `x.copy()` when the function promises purity.
- **Numerical stability.** `np.exp(1000)` overflows to `inf`. Softmax is the
  canonical case: subtract `max(logits)` first. This shifts every exponent
  into `(-inf, 0]`, so `exp` can't overflow — and the result is unchanged
  because softmax(x) == softmax(x - c) for any constant c (the e^{-c}
  factors cancel in numerator and denominator). Interviewers specifically
  probe for this.

## Drills

| Drill | Idiom it teaches | Java equivalent |
|---|---|---|
| `normalize` | `v / np.linalg.norm(v)` — whole-array division | loop computing sumsq, then loop dividing |
| `relu` | `np.maximum(x, 0)` elementwise, no loop | `for i: out[i] = Math.max(x[i], 0)` |
| `softmax` | subtract max for stability, then `exp` / `sum` | loop + manual overflow bugs |
| `one_hot` | fancy indexing: `np.eye(k)[labels]` | nested loop filling `double[n][k]` |
| `count_above` | boolean mask + `.sum()` | counter loop with `if (x[i] > t) count++` |
| `clamp_negatives` | mask assignment on a `.copy()` | loop with `Math.max`, or accidental in-place mutation |
| `row_argmax` | `argmax(axis=1)` — the axis mental model | per-row loop tracking bestIdx |
| `moving_average` | `np.convolve(..., mode="valid") / k` | sliding-window loop with running sum |
| `pairwise_distances` | broadcasting: `a[:, None, :] - b[None, :, :]` | triple nested loop over n, m, d |
| `standardize_columns` | column stats via `axis=0`, `np.where` zero-std guard | column-major loops + divide-by-zero checks |

## Run it

```bash
# against the reference solutions
uv run pytest ramp_up/python/08_numpy_essentials -v

# against your own attempt in starter.py
PRACTICE=1 uv run pytest ramp_up/python/08_numpy_essentials -v
```
