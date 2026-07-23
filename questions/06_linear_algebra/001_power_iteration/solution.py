"""Power iteration for the dominant eigenpair.

Fixed-seed random start: deterministic, and (unlike a ones-vector) almost
surely not orthogonal to the dominant eigenvector. Convergence compares
against both +v and -v so matrices with a negative dominant eigenvalue
(where the iterate flips sign every step) still terminate.
"""

import numpy as np


def rayleigh_quotient(A: np.ndarray, v: np.ndarray) -> float:
    """Eigenvalue estimate v^T A v / v^T v for a nonzero vector v."""
    A = np.asarray(A, dtype=float)
    v = np.asarray(v, dtype=float)
    denom = v @ v
    if denom == 0.0:
        raise ValueError("v must be nonzero")
    return float(v @ A @ v / denom)


def power_iteration(
    A: np.ndarray, num_iters: int = 1000, tol: float = 1e-10
) -> tuple[float, np.ndarray]:
    """Dominant eigenpair of square matrix A. Eigenvector is unit length."""
    A = np.asarray(A, dtype=float)
    n = A.shape[0]
    if A.shape != (n, n):
        raise ValueError("A must be square")

    rng = np.random.default_rng(0)
    v = rng.standard_normal(n)
    v /= np.linalg.norm(v)

    for _ in range(num_iters):
        w = A @ v
        norm = np.linalg.norm(w)
        if norm == 0.0:
            # v is in A's null space; 0 is the eigenvalue for this direction.
            return 0.0, v
        w /= norm
        # Sign-insensitive convergence: with a negative dominant eigenvalue
        # the direction is fixed but the sign alternates each step.
        if min(np.linalg.norm(w - v), np.linalg.norm(w + v)) < tol:
            v = w
            break
        v = w

    return rayleigh_quotient(A, v), v
