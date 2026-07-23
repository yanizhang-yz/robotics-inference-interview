"""
Power Iteration — YOUR ATTEMPT

Write your solution here, then run the tests against it:
    PRACTICE=1 uv run pytest questions/06_linear_algebra/001_power_iteration -v

Peek at solution.py only after you've tried.
"""

import numpy as np


def rayleigh_quotient(A: np.ndarray, v: np.ndarray) -> float:
    """
    Eigenvalue estimate for direction v: (v @ A @ v) / (v @ v).

    Raise ValueError for a zero vector.
    """
    # TODO: implement
    raise NotImplementedError


def power_iteration(
    A: np.ndarray, num_iters: int = 1000, tol: float = 1e-10
) -> tuple[float, np.ndarray]:
    """
    Return (dominant eigenvalue, unit eigenvector) of square matrix A.

    Plan:
    1. Start from a FIXED-SEED random unit vector:
       np.random.default_rng(0).standard_normal(n), normalized.
    2. Repeat: w = A @ v, normalize w (guard: if ||w|| == 0, v is in the
       null space -> return (0.0, v)).
    3. Stop when the direction stops changing. Compare against BOTH v and
       -v (min of the two norms < tol) — a negative dominant eigenvalue
       flips the sign every step even after the direction has converged.
    4. Eigenvalue = rayleigh_quotient(A, final_v).

    Raise ValueError if A is not square.
    """
    # TODO: implement
    raise NotImplementedError
