import math
import numpy as np


def Rx(angle_deg: float) -> np.ndarray:
    """Rotation matrix about the X axis."""
    t = math.radians(angle_deg)
    c, s = math.cos(t), math.sin(t)
    return np.array([[1, 0,  0],
                     [0, c, -s],
                     [0, s,  c]], dtype=float)


def Ry(angle_deg: float) -> np.ndarray:
    """Rotation matrix about the Y axis."""
    t = math.radians(angle_deg)
    c, s = math.cos(t), math.sin(t)
    return np.array([[ c, 0, s],
                     [ 0, 1, 0],
                     [-s, 0, c]], dtype=float)


def Rz(angle_deg: float) -> np.ndarray:
    """Rotation matrix about the Z axis."""
    t = math.radians(angle_deg)
    c, s = math.cos(t), math.sin(t)
    return np.array([[c, -s, 0],
                     [s,  c, 0],
                     [0,  0, 1]], dtype=float)


def rotate_point(point: list[float], R: np.ndarray) -> np.ndarray:
    """Apply rotation matrix R to a 3D point."""
    return R @ np.array(point, dtype=float)


def is_valid_rotation(R: np.ndarray, tol: float = 1e-9) -> bool:
    """Check that R is in SO(3): orthogonal with determinant +1."""
    identity_check = np.allclose(R.T @ R, np.eye(3), atol=tol)
    det_check = abs(np.linalg.det(R) - 1.0) < tol
    return identity_check and det_check
