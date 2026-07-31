"""Least squares two ways: normal equations by hand, then np.linalg.lstsq.

The normal equations A^T A x = A^T b pick the x whose residual b - Ax is
orthogonal to every column of A — i.e. they project b onto A's column
space. Forming A^T A squares the condition number of A (the ratio of its
largest to smallest singular value — how much a solve can amplify input
error), which is fine for a well-scaled 2x2 but is why the QR/SVD route
inside lstsq is the numerically safer general tool. Both are shown here
on purpose.
"""

import numpy as np


def fit_line(xs, ys) -> tuple[float, float]:
    """(slope, intercept) minimizing squared error, via the normal equations."""
    xs = np.asarray(xs, dtype=float)
    ys = np.asarray(ys, dtype=float)
    if xs.size < 2:
        raise ValueError("need at least 2 points to fit a line")

    A = np.column_stack([xs, np.ones(xs.size)])
    # Normal equations: (A^T A) params = A^T ys — a 2x2 solve.
    slope, intercept = np.linalg.solve(A.T @ A, A.T @ ys)
    return float(slope), float(intercept)


def fit_poly(xs, ys, degree: int) -> np.ndarray:
    """Polynomial coefficients, highest power first (np.polyfit order)."""
    xs = np.asarray(xs, dtype=float)
    ys = np.asarray(ys, dtype=float)
    if xs.size < degree + 1:
        raise ValueError(
            f"need at least {degree + 1} points for degree {degree}, got {xs.size}"
        )

    A = np.vander(xs, degree + 1)  # columns x^degree ... x^0, highest first
    # np.linalg.lstsq, NOT np.polyfit: lstsq is the general least-squares
    # solver (QR/SVD-based, no condition-number squaring); polyfit is the
    # convenience wrapper around it that this exercise reimplements.
    coeffs, *_ = np.linalg.lstsq(A, ys, rcond=None)
    return coeffs


def predict(coeffs, xs) -> np.ndarray:
    """Evaluate the polynomial at xs; coeffs are highest power first."""
    coeffs = np.asarray(coeffs, dtype=float)
    xs = np.asarray(xs, dtype=float)
    # Same design matrix as the fit, times the coefficients.
    return np.vander(xs, coeffs.size) @ coeffs
