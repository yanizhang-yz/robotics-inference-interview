# Power Iteration

**Difficulty:** Medium  
**Source:** Classic numerical linear algebra question — asked in ML/perception interviews as "find the dominant eigenvector without calling eig"  
**Tags:** `eigenvector`, `iterative-methods`, `numpy`, `numerical-stability`

## Problem

Given a square matrix `A`, find its **dominant eigenpair** — the eigenvalue with the
largest absolute value, and its eigenvector — *without* calling `np.linalg.eig`:

- `power_iteration(A, num_iters=1000, tol=1e-10) -> (eigenvalue, eigenvector)`
  Returns a unit-length eigenvector and its eigenvalue. Stops early once the
  vector stops changing (within `tol`).
- `rayleigh_quotient(A, v) -> float` — the eigenvalue estimate `vᵀAv / vᵀv` for any
  nonzero vector `v`.

```
A = [[2, 0],
     [0, 1]]
power_iteration(A)   # -> (2.0, [1.0, 0.0])   (up to sign)
```

Note the "up to sign": if `v` is an eigenvector, so is `-v`. Tests must compare
directions, not raw components.

## Why this appears in robotics inference interviews

Power iteration is the smallest possible window into how the big tools actually work:
PCA's principal component *is* the dominant eigenvector of the covariance matrix,
PageRank runs power iteration on the web's link matrix, and spectral methods in SLAM
and point-cloud registration reduce to dominant eigenpairs. Interviewers use it to
check three things at once: do you know what an eigenvector *is* (a direction the
matrix only stretches), can you write a stable iterative loop (normalize every step,
define convergence), and do you handle the numerical edges (zero vectors, sign flips).

## Approach

The idea: multiplying any vector by `A` stretches it most along the dominant
eigenvector's direction. Repeat, and that direction wins.

1. Start from a fixed-seed random vector (deterministic, and almost surely not
   orthogonal to the dominant eigenvector — a ones-vector can be).
2. Loop: `v_next = A @ v`, then **normalize** (`v_next /= ||v_next||`) — without this
   the values overflow or vanish.
3. Converged when `||v_next - v||` (compare against both `v` and `-v` to tolerate
   sign flips for negative eigenvalues) drops below `tol`.
4. Eigenvalue = Rayleigh quotient of the final vector.

**Time:** O(n² · iters) for dense A. **Space:** O(n).

## Follow-ups to be ready for

1. Convergence *rate* depends on the gap `|λ₂|/|λ₁|` — near-ties converge slowly; what
   breaks when `|λ₁| == |λ₂|` exactly?
2. Second eigenvector: **deflation** (subtract `λ₁ v₁v₁ᵀ`) or orthogonalize each step.
3. Smallest eigenvalue: inverse iteration (power iteration on `A⁻¹`, solved as a
   linear system each step).
4. How PCA uses this: dominant eigenvector of `XᵀX/n` = direction of maximum variance.
