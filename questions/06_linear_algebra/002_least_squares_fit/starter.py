"""
Least Squares Fit — YOUR ATTEMPT

Write your solution here, then run the tests against it:
    PRACTICE=1 uv run pytest questions/06_linear_algebra/002_least_squares_fit -v

Peek at solution.py only after you've tried.
"""

import numpy as np


def fit_line(xs, ys) -> tuple[float, float]:
    """
    Return (slope, intercept) minimizing the sum of squared errors.

    Plan:
    1. Design matrix A = np.column_stack([xs, np.ones(len(xs))]) — each row
       [x_i, 1], so A @ [slope, intercept] stacks slope*x_i + intercept.
    2. Normal equations: solve (A.T @ A) params = A.T @ ys with
       np.linalg.solve. (Why they work: they pick the params whose residual
       is orthogonal to A's columns — the projection of ys onto A's column
       space.)
    3. Return (float(slope), float(intercept)).

    Raise ValueError for fewer than 2 points.
    """
    # TODO: implement
    raise NotImplementedError


def fit_poly(xs, ys, degree: int) -> np.ndarray:
    """
    Return polynomial coefficients, HIGHEST power first (np.polyfit order).

    Plan:
    1. Raise ValueError when len(xs) < degree + 1 (underdetermined system).
    2. A = np.vander(xs, degree + 1) — columns x^degree ... x^0, highest
       power first.
    3. coeffs, *_ = np.linalg.lstsq(A, ys, rcond=None)
       Use lstsq, NOT np.polyfit: lstsq is the general least-squares solver
       (QR/SVD under the hood, no condition-number squaring); polyfit is
       the convenience wrapper around it that you are reimplementing here.
    """
    # TODO: implement
    raise NotImplementedError


def predict(coeffs, xs) -> np.ndarray:
    """
    Evaluate the polynomial at xs; coeffs are highest power first.

    Plan: np.vander(xs, len(coeffs)) @ coeffs — the same design matrix
    times the coefficients. (np.polyval computes the same thing via
    Horner's rule.) Works for fit_line's (slope, intercept) pair too.
    """
    # TODO: implement
    raise NotImplementedError
