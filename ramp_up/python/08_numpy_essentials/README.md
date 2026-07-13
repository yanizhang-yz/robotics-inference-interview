# 08 — NumPy essentials

After this lesson you will be able to replace Java-style element loops with
whole-array NumPy operations: filter and count with boolean masks, reduce a
matrix along a chosen axis, build lookup tables with fancy indexing, and
combine differently-shaped arrays with broadcasting. These are the exact
moves you need to implement softmax, normalization, one-hot encoding, and
distance matrices live in an interview — which is most of the NumPy an
inference-engineer interview actually asks for.

## The Java you know

In Java, "do X to every element" means a `for` loop over `double[]`. That is
the only idiom, and it is fine — the JIT compiles it to fast machine code:

```java
// Scale a vector to unit length: two passes, index arithmetic, manual guard.
static double[] normalize(double[] v) {
    double sumSq = 0.0;
    for (int i = 0; i < v.length; i++) {
        sumSq += v[i] * v[i];
    }
    double norm = Math.sqrt(sumSq);
    double[] out = new double[v.length];
    if (norm == 0.0) return out;              // all zeros
    for (int i = 0; i < v.length; i++) {
        out[i] = v[i] / norm;                 // -> [0.6, 0.8] for {3.0, 4.0}
    }
    return out;
}

// Count elements above a threshold: the counter-loop idiom.
static int countAbove(double[] x, double t) {
    int count = 0;
    for (int i = 0; i < x.length; i++) {
        if (x[i] > t) count++;                // -> 3 for {1,5,3,7}, t=2.5
    }
    return count;
}
```

In NumPy those two functions are `v / np.linalg.norm(v)` and
`(x > t).sum()` — one line each, no loop. This lesson explains why that
works, and why in Python the loop version is well over 100x slower.

## The lesson

### What an ndarray actually is (and why loops are slow in Python)

NumPy's core type is the **ndarray** ("n-dimensional array"). Unlike a
Python `list` — which is a resizable bag of pointers to arbitrary objects —
an ndarray is a single fixed-size block of raw memory, like Java's
`double[]`. Every element has the same type, recorded once for the whole
array as the **dtype** (data type, e.g. `float64` = Java `double`,
`int64` = Java `long`). An array where all elements share one type is
called **homogeneous**; a Python list, which can mix types
(`[1, "two", 3.0]`), is **heterogeneous**.

```python
import numpy as np
a = np.array([1, 2, 3]);      a.dtype    # -> dtype('int64')
b = np.array([1.0, 2.0]);     b.dtype    # -> dtype('float64')
c = np.array([1, 2.5]);       c          # -> array([1. , 2.5])  ints promoted to float
```

Gotcha: the dtype is fixed at creation. If you assign a float into an
int array, NumPy silently truncates it — `a[0] = 9.7` on an `int64` array
stores `9`, no error, no warning.

Why loops are slow: Python is interpreted, so every `x[i]` in a `for`
loop pays interpreter overhead — type checks, object wrapping, method
dispatch — per element. A whole-array operation does the loop once, in
compiled C, over the raw memory block. Measured on a million elements: a
Python loop summing them takes ~29 ms; `arr.sum()` takes ~0.1 ms. (Bonus:
NumPy's C loops release the GIL — Python's global interpreter lock — so
they can run in parallel; see Gotcha 12 in `../LEARNING_POINTS.md`.)

**The mental shift: stop thinking "for each element", start thinking
"which whole-array operation expresses this?"** In this module, a `for`
loop over array elements is a bug.

### Vectorization: arithmetic on whole arrays

**Vectorization** means writing an operation against the whole array and
letting NumPy apply it to every element in C. All the arithmetic operators
are **elementwise** (applied independently to each element):

```java
// Java
double[] out = new double[x.length];
for (int i = 0; i < x.length; i++) out[i] = x[i] * 2;
```

```python
# Python
x = np.array([1.0, 2.0, 3.0])
x * 2          # -> array([2., 4., 6.])
x + x          # -> array([2., 4., 6.])
x ** 2         # -> array([1., 4., 9.])
np.sqrt(x)     # elementwise sqrt — note np.sqrt, not math.sqrt
```

Two gotchas: this is NumPy behavior, not Python behavior — on a plain
list, `[1, 2, 3] * 2` gives `[1, 2, 3, 1, 2, 3]` (repetition), while
`np.array([1, 2, 3]) * 2` gives `[2, 4, 6]`; if your "math" looks insane,
check whether you are holding a list or an array. And `math.sqrt` /
`math.exp` accept only single numbers (`TypeError` on arrays) — always
reach for the `np.` versions, which handle both.

### Boolean masks: filtering and counting without loops

In Java, `x[i] > 0` is a `boolean`. In NumPy, `x > 0` compares every
element at once and returns a **boolean mask** — an array of
`True`/`False` the same shape as `x`:

```python
x = np.array([1.0, -2.0, 3.0, -4.0])
mask = x > 0        # -> array([ True, False,  True, False])
```

One mask replaces three Java loop patterns:

```python
mask.sum()          # COUNT:  True counts as 1, False as 0     -> 2
x[mask]             # FILTER: keeps elements where mask is True -> array([1., 3.])
x[x < 0] = 0        # ASSIGN: writes only where mask is True
                    #         x is now array([1., 0., 3., 0.])
```

Gotcha: to combine conditions use `&` and `|` with parentheses (they bind
tighter than `>`), not `and`/`or` — `(x > 0) and (x < 3)` raises
`ValueError: The truth value of an array ... is ambiguous`, because
Python's `and` demands one single yes/no and a mask has many. Write
`(x > 0) & (x < 3)`.

Gotcha: reductions return NumPy scalar types — `mask.sum()` is a
`numpy.int64`, and `isinstance(mask.sum(), int)` is `False`. When a
function promises a plain `int`, wrap it: `int(mask.sum())`.

### The axis mental model: which dimension disappears

A **reduction** collapses many values into fewer — `sum`, `mean`, `max`,
`argmax` are all reductions. On a matrix, the **axis** argument says
*which dimension to collapse*. For a 2-D array, `axis=0` is the row
dimension (down the columns) and `axis=1` is the column dimension (across
each row).

The one rule that removes all confusion: **the axis you pass is the one
that disappears from the result's shape.**

```python
m = np.array([[1, 2, 3],
              [4, 5, 6]])       # shape (2, 3)

m.sum()          # no axis: collapse everything      -> 21
m.sum(axis=0)    # collapse the 2 rows               -> array([5, 7, 9])   shape (3,) — one per COLUMN
m.sum(axis=1)    # collapse the 3 columns            -> array([ 6, 15])    shape (2,) — one per ROW
m.mean(axis=0)   # per-column means                  -> array([2.5, 3.5, 4.5])
```

Java equivalent of `m.sum(axis=0)`: nested loops accumulating into
`double[] sums = new double[cols]`. When unsure which axis you need, do
not guess — check the result shape: `(2, 3)` with `axis=0` must give
`(3,)`; if you wanted one value per row, that was the wrong axis.

### Fancy indexing: an array as the index

Java array indices are single `int`s. NumPy also accepts an *array of
indices* — called **fancy indexing** — which gathers all those positions
in one shot, in order, with repeats allowed:

```python
arr = np.array([10, 20, 30, 40])
arr[np.array([3, 0, 0])]     # -> array([40, 10, 10])
```

On a 2-D array, a 1-D integer index picks whole *rows*. That turns
one-hot encoding into a table lookup. **One-hot encoding** represents a
class label `i` (out of `k` classes) as a length-`k` vector of zeros with
a single 1.0 at position `i` — and row `i` of the `k x k` **identity
matrix** (ones on the diagonal, `np.eye(k)`) is exactly that vector:

```python
np.eye(3)[np.array([0, 2, 1])]
# -> array([[1., 0., 0.],      row 0 of eye(3) = one-hot for label 0
#           [0., 0., 1.],      row 2            = one-hot for label 2
#           [0., 1., 0.]])     row 1            = one-hot for label 1
```

Gotcha: fancy indexing returns a **copy** (a new independent array);
plain slicing like `arr[1:3]` returns a **view** (a window onto the same
memory). More on that below.

### Broadcasting: combining arrays of different shapes

**Broadcasting** is NumPy's rule for arithmetic between arrays whose
shapes differ. Align the shapes from the *right*; two dimensions are
compatible if they are equal, or if either is 1 — a size-1 dimension is
"stretched" (values reused, nothing actually copied) to match the other:

```python
col = np.array([[1], [2], [3]])     # shape (3, 1)
row = np.array([10, 20, 30, 40])    # shape (4,)  — treated as (1, 4)
col + row                           # shapes (3,1)+(1,4) -> result (3, 4)
# -> array([[11, 21, 31, 41],
#           [12, 22, 32, 42],
#           [13, 23, 33, 43]])
```

Incompatible shapes fail loudly: `np.zeros(3) + np.zeros(4)` raises
`ValueError: operands could not be broadcast together with shapes (3,) (4,)`.

You *set up* broadcasting deliberately by inserting new size-1 dimensions
with `None` (an alias for `np.newaxis`) inside an index:

```python
v = np.array([1.0, 2.0, 3.0])   # shape (3,)
v[:, None].shape                # -> (3, 1)   column vector
v[None, :].shape                # -> (1, 3)   row vector
```

The payoff — every pairwise difference between an `(n, d)` and an
`(m, d)` set of points, with zero loops:

```python
a[:, None, :] - b[None, :, :]   # (n,1,d) - (1,m,d) -> (n, m, d)
```

Entry `[i, j]` of the result is the vector `a[i] - b[j]`. In Java this is
a triple nested loop over `n`, `m`, `d`. This one line is the single most
tested NumPy idiom in ML-infrastructure interviews.

### Views vs copies: the mutation trap

Python variables are references, like Java object references —
`List<Integer> b = a` makes `b` and `a` the same list. NumPy adds a
second trap: **slicing returns a view**, a window onto the *same
underlying memory*, not a **copy** (a new independent array — what Java's
`Arrays.copyOfRange` gives you).

```python
x = np.array([1.0, 2.0, 3.0, 4.0])
s = x[1:3]        # view — no data copied
s[0] = 99.0
x                 # -> array([ 1., 99.,  3.,  4.])   x changed!
```

And plain assignment never copies anything:

```python
x = np.array([-1.0, 2.0])
out = x           # out IS x — same array, two names
out[out < 0] = 0
x                 # -> array([0., 2.])   caller's array clobbered
```

A function that promises not to modify its input (a "pure" function) must
copy first: `out = x.copy()`. The rule of thumb: **slices are views;
fancy indexing and boolean-mask indexing return copies; when in doubt,
`.copy()`.** This is NumPy's version of the shallow-copy trap — see
Gotcha 9 in `../LEARNING_POINTS.md`.

### Numerical stability: when the math is right but the floats overflow

`float64` (Java `double`) maxes out around `1.8e308`. `np.exp(710.0)`
exceeds that, so the result **overflows** to `inf` — a special float value
meaning "too big to represent". And `inf / inf` is **NaN** ("not a
number", the poison value that makes every comparison false and spreads
through all arithmetic — same as Java's `Double.NaN`).

Softmax is the canonical victim. Softmax turns a vector of raw scores —
called **logits** in ML — into probabilities that sum to 1:
`exp(x_i) / sum(exp(x))`. The naive version explodes on big inputs:

```python
logits = np.array([1000.0, 1000.0])
e = np.exp(logits)              # -> array([inf, inf])   overflow
e / e.sum()                     # -> array([nan, nan])   inf/inf

e = np.exp(logits - logits.max())   # exponents now <= 0, exp() <= 1
e / e.sum()                          # -> array([0.5, 0.5])   correct
```

Why subtracting the max is legal: `softmax(x) == softmax(x - c)` for any
constant `c`, because the `e^{-c}` factors cancel between numerator and
denominator. After subtracting the max, every exponent is `<= 0`, so
`exp()` lands in `(0, 1]` and cannot overflow. **Interviewers specifically
probe for this** — writing the naive softmax and getting NaN on large
logits is a known screen-out.

## Muscle memory

Type these without thinking:

```python
np.maximum(x, 0)                  # elementwise max vs scalar (relu)
(x > t).sum()                     # count matching elements
x[x < 0] = 0                      # mask assignment (on a copy!)
x.copy()                          # before any in-place edit of a parameter
mat.mean(axis=0)                  # per-column stats  (axis 0 disappears)
mat.argmax(axis=1)                # per-row winner    (axis 1 disappears)
np.eye(k)[labels]                 # one-hot via fancy indexing
a[:, None, :] - b[None, :, :]     # all-pairs differences via broadcasting
np.linalg.norm(v)                 # Euclidean length: sqrt(sum(v**2))
np.where(cond, a, b)              # elementwise if/else, e.g. zero-div guards
```

## The drills

Work top to bottom in `starter.py`. None of them needs a loop.

### `normalize`

Scale a vector to unit length; return all zeros for the zero vector (no
NaN). The **Euclidean norm** (or L2 norm) is the geometric length
`sqrt(sum(v_i^2))`; dividing by it makes length exactly 1.

```python
v = np.array([3.0, 4.0])
np.linalg.norm(v)            # -> 5.0
v / np.linalg.norm(v)        # -> array([0.6, 0.8])
```

Guard the zero vector with an `if norm == 0` returning
`np.zeros_like(v, dtype=float)` — otherwise `0/0` produces NaN.

Where you'll see it: "implement cosine similarity" (normalize, then dot
product) — a standard ML screen, and the core of vector search over
embeddings (the learned vector representations models produce for text
and images). In robotics: direction vectors, quaternion normalization
before rotation math. The zero-vector guard is what interviewers watch for.

### `relu`

Elementwise `max(x, 0)`, any shape, no loop. ReLU ("rectified linear
unit") is the most common neural-network activation function.

```python
np.maximum(np.array([-2.0, 0.0, 3.5]), 0)   # -> array([0. , 0. , 3.5])
```

Note `np.maximum` (elementwise, two args, the scalar 0 broadcasts) vs
`np.max` (a reduction that collapses one array to its largest value) —
classic mix-up.

Where you'll see it: "implement an MLP forward pass by hand" (MLP =
multi-layer perceptron, the basic neural network) and other
NN-from-scratch interview rounds; every inference pipeline you will read
has these one-liners between layers.

### `softmax`

Logits to probabilities, numerically stable for logits like 1000.

```python
logits = np.array([1.0, 2.0, 3.0])
e = np.exp(logits - logits.max())
e / e.sum()                  # -> array([0.09 , 0.245, 0.665])  (rounded; sums to 1)
```

Where you'll see it: THE classic ML-infra whiteboard question —
"implement softmax; now make it stable". It is the last operation of
every classification model, and the middle of attention ("implement
scaled dot-product attention" contains a softmax over the score matrix).
Expect the interviewer to feed your version large logits on purpose.

### `one_hot`

`(len(labels), num_classes)` matrix of 0.0/1.0 with row `i` hot at column
`labels[i]`.

```python
np.eye(3)[np.array([0, 2, 1])]
# -> [[1,0,0], [0,0,1], [0,1,0]]  (as floats)
```

Where you'll see it: "implement cross-entropy loss from scratch" —
one-hot the labels, multiply elementwise with the log-probabilities, sum;
label preprocessing in any training pipeline. Interviewers like it
because the loop version is 5 lines and the fancy-indexing version is
1 — it reveals whether you think in arrays.

### `count_above`

Count elements strictly greater than a threshold; return a plain `int`.

```python
x = np.array([1.0, 5.0, 3.0, 7.0])
int((x > 2.5).sum())         # -> 3
```

The `int(...)` matters: `.sum()` returns `numpy.int64`, and
`isinstance(np.int64(0), int)` is `False` — the tests check the type.

Where you'll see it: buried inside bigger problems — accuracy is
`(preds == labels).sum() / len(labels)`, the same idiom with `==`;
detection post-processing counts/keeps boxes above a confidence
threshold. If you write a counter loop here in an interview, it signals
you have not used NumPy in anger.

### `clamp_negatives`

Return a copy with negatives set to 0 — the input must not be mutated
(the tests check).

```python
out = x.copy()
out[out < 0] = 0
```

Without `.copy()` you clobber the caller's array through the alias — the
single most common NumPy bug in review. The general tool is
`np.clip(x, lo, hi)`: `np.clip([-5, 128, 300], 0, 255)` -> `[0, 128, 255]`.

Where you'll see it: image post-processing (pixel values to `[0, 255]`),
gradient clipping, action clamping to joint limits in robot control. The
interview probe is rarely the clamp itself — it is "does your function
mutate its input?" asked afterward.

### `row_argmax`

For a 2-D matrix, the column index of each row's maximum, shape
`(n_rows,)`. **argmax** = "index of the max" (not the max value).

```python
mat = np.array([[1, 9, 3],
                [7, 2, 5]])
mat.argmax(axis=1)           # -> array([1, 0])
```

`axis=1` because the column dimension is the one that must disappear —
one answer per row. Ties return the first index.

Where you'll see it: decoding predictions — "which class did the model
pick per sample" is `probs.argmax(axis=1)` in every eval script; greedy
decoding in sequence models; the assignment step of "implement k-means"
(each point goes to its argmin-distance centroid — same idiom, `argmin`).

### `moving_average`

Window-`k` moving average, "valid" mode: output length `len(x) - k + 1`,
each entry the mean of one full window. **Convolution** here just means:
slide a window across the signal, multiply elementwise, sum — convolving
with `np.ones(k)` makes each output a window *sum*, so divide by `k`.

```python
x = np.array([1.0, 2.0, 3.0, 4.0])
np.convolve(x, np.ones(2), mode="valid")       # -> array([3., 5., 7.])  window sums
np.convolve(x, np.ones(2), mode="valid") / 2   # -> array([1.5, 2.5, 3.5])
```

`mode="valid"` keeps only windows that fully overlap the signal (no
zero-padded edges).

Where you'll see it: "Moving Average from Data Stream" and the
sliding-window family ("Sliding Window Maximum", "Maximum Average
Subarray") on LeetCode; in robotics it is the first thing you do to any
noisy sensor stream — smoothing IMU (inertial measurement unit),
joint-torque, or force readings before acting on them.

### `pairwise_distances`

`a` is `(n, d)`, `b` is `(m, d)`; return the `(n, m)` matrix of Euclidean
distances between every `a[i]` and `b[j]`. THE broadcasting drill.

```python
diff = a[:, None, :] - b[None, :, :]   # (n, m, d): diff[i, j] = a[i] - b[j]
np.sqrt((diff ** 2).sum(axis=-1))      # square, sum over d, sqrt -> (n, m)
# for a=[[0,0],[1,0]], b=[[0,3],[4,0],[0,0]]:
# -> [[3.   , 4.   , 0.   ],
#     [3.162, 3.   , 1.   ]]   (3.162 = sqrt(10), rounded)
```

`axis=-1` means "the last axis" — here the coordinate dimension `d`,
which is the one to collapse.

Where you'll see it: "implement k-nearest neighbors" and "implement
k-means" both reduce to exactly this matrix; "K Closest Points to Origin"
is the 1-D special case. In robotics: nearest-obstacle queries, and the
correspondence step of point-cloud alignment (ICP — iterative closest
point). This is the #1 "do you actually know broadcasting" interview
test.

### `standardize_columns`

Standardize each column to mean 0, standard deviation 1; constant columns
become all zeros (not NaN); do not mutate the input. **Standardizing**
(also "z-scoring") means subtracting the mean and dividing by the
**standard deviation** (the average spread around the mean) so all
features live on the same scale.

```python
mean = mat.mean(axis=0)                    # per-column, shape (n_cols,)
std = mat.std(axis=0)
(mat - mean) / np.where(std == 0, 1.0, std)
```

Two ideas combine: `axis=0` stats broadcast back over the rows on
subtraction, and `np.where(cond, a, b)` — an elementwise if/else — swaps
each zero std for 1.0 so constant columns divide by 1 (their numerator is
already all zeros, so they standardize to zeros instead of NaN).

Where you'll see it: feature preprocessing in every classical-ML
pipeline; "implement batch normalization" is this exact computation per
feature plus two learned parameters; interviewers use the zero-std column
as the hidden edge case, the same way they use empty arrays.

## How to practice

Write your attempts in `starter.py`, then run the tests against them:

```bash
PRACTICE=1 uv run pytest ramp_up/python/08_numpy_essentials -v
```

Drop the `PRACTICE=1` to run the tests against `solution.py` instead.
