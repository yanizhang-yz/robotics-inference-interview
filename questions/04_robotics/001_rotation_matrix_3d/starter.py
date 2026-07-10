"""
3D Rotation Matrix — YOUR ATTEMPT

Write your solution here, then run the tests against it:
    PRACTICE=1 uv run pytest questions/04_robotics/001_rotation_matrix_3d -v

Peek at solution.py only after you've tried.
"""

import numpy as np


def Rx(angle_deg: float) -> np.ndarray:
    """Rotation matrix about the X axis."""
    # TODO: implement
    raise NotImplementedError


def Ry(angle_deg: float) -> np.ndarray:
    """Rotation matrix about the Y axis."""
    # TODO: implement
    raise NotImplementedError


def Rz(angle_deg: float) -> np.ndarray:
    """Rotation matrix about the Z axis."""
    # TODO: implement
    raise NotImplementedError


def rotate_point(point: list[float], R: np.ndarray) -> np.ndarray:
    """Apply rotation matrix R to a 3D point."""
    # TODO: implement
    raise NotImplementedError


def is_valid_rotation(R: np.ndarray, tol: float = 1e-9) -> bool:
    """
    Check that R is in SO(3).

    Hint: orthogonal (R.T @ R == I) and det(R) == +1.
    """
    # TODO: implement
    raise NotImplementedError
