# Least Squares Fit (Two Ways)

**Difficulty:** Easy–Medium  
**Source:** Standard applied-linear-algebra screener across perception and calibration teams — asked so broadly that no single-company attribution would be honest  
**Tags:** `least-squares`, `normal-equations`, `lstsq`, `conditioning`, `calibration`

## Problem

Fit models to noisy data, two ways:

- `fit_line(xs, ys) -> (slope, intercept)` — minimize the sum of squared errors via
  the **normal equations** on the design matrix whose rows are `[x_i, 1]`.
- `fit_poly(xs, ys, degree) -> coefficients` — highest power first (matching
  `np.polyfit`), via **`np.linalg.lstsq`**. Raise `ValueError` when
  `len(xs) < degree + 1` (underdetermined).
- `predict(coeffs, xs)` — evaluate the fitted polynomial (works for the line's
  `(slope, intercept)` pair too: it's just a degree-1 coefficient vector).

```python
xs = [0, 1, 2, 3]
ys = [1.1, 2.9, 5.2, 6.8]
fit_line(xs, ys)          # ≈ (1.94, 1.09) — minimizes Σ (y - (m·x + b))²
```

## Why this appears in robotics inference interviews

Fitting lines and planes *is* daily robotics work: refining camera intrinsics,
estimating the ground plane from a point cloud, calibrating wheel odometry against
ground truth, trend-fitting latency curves from profiler logs. Interviewers use it
to check two things: that you can set up `Ax = b` from a word problem (choosing the
design matrix is the actual skill), and that you know *why* the normal equations
`AᵀA x = Aᵀb` work — they pick the coefficients that **project b onto A's column
space**, the closest point to `b` that the model can express. It's the same
linear-algebra muscle as the power-iteration question next door
(`001_power_iteration`): that one finds the direction a matrix stretches most; this
one finds the combination of columns that lands nearest `b`.

## Approach

1. Build the design matrix: `np.column_stack([xs, np.ones(len(xs))])` for the line;
   `np.vander(xs, degree + 1)` for the polynomial — columns `x^degree … x^0`,
   highest power first, which is exactly `np.polyfit`'s coefficient order.
2. Line: solve the 2×2 normal equations directly — `np.linalg.solve(A.T @ A, A.T @ ys)`.
3. Polynomial: `np.linalg.lstsq(A, ys, rcond=None)`. Not `np.polyfit` — `lstsq` is
   the general least-squares solver; `polyfit` is the convenience wrapper around it
   that this exercise reimplements.

**Conditioning caveat:** forming `AᵀA` **squares the condition number** of `A` (the
condition number — the ratio of a matrix's largest to smallest singular value — is a
measure of how much a solve can amplify input error). Fine for a well-scaled 2×2;
for higher degrees or badly scaled data, the QR/SVD route inside `lstsq` is the
numerically safer tool. Both appear here deliberately: normal equations to show you
know the math, `lstsq` to show you know the tool.

**Time:** O(n·d²) for the factorization, with `d = degree + 1`.  
**Space:** O(n·d) for the design matrix.

## Follow-ups to be ready for

1. **RANSAC** — the real-world robotics answer: ground planes come with outliers
   (curbs, pedestrians, sensor noise) and plain least squares chases them; fit on
   random minimal subsets and keep the model with the largest consensus set.
2. **Ridge regularization one-liner:** solve `(AᵀA + λI) x = Aᵀb` — the `λI` floor
   fixes a near-singular `AᵀA` and shrinks the coefficients.
3. **Fitting a plane to 3D points:** design matrix rows `[x, y, 1]` for
   `z = ax + by + c`; for a fully general plane, take the smallest-eigenvector of
   the centered covariance — power iteration's sibling problem.
4. **Weighted least squares:** minimize `Σ wᵢ rᵢ²` by scaling each row of `A` and
   `b` by `√wᵢ` — the right tool when sensors have different noise levels.
