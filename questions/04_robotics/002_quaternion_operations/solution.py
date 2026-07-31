"""Quaternion operations, scalar-first convention (w, x, y, z).

quat_to_rotation_matrix normalizes internally so non-unit inputs (e.g. a
drifted integrated IMU quaternion) still yield a valid member of SO(3).
"""

import numpy as np


def quat_multiply(q1, q2) -> np.ndarray:
    """Hamilton product q1 ⊗ q2 (apply q2's rotation first, then q1's)."""
    w1, x1, y1, z1 = np.asarray(q1, dtype=float)
    w2, x2, y2, z2 = np.asarray(q2, dtype=float)
    return np.array([
        w1 * w2 - x1 * x2 - y1 * y2 - z1 * z2,
        w1 * x2 + x1 * w2 + y1 * z2 - z1 * y2,
        w1 * y2 - x1 * z2 + y1 * w2 + z1 * x2,
        w1 * z2 + x1 * y2 - y1 * x2 + z1 * w2,
    ])


def quat_conjugate(q) -> np.ndarray:
    """Conjugate (w, -x, -y, -z); the inverse for a unit quaternion."""
    w, x, y, z = np.asarray(q, dtype=float)
    return np.array([w, -x, -y, -z])


def quat_to_rotation_matrix(q) -> np.ndarray:
    """3x3 rotation matrix for q. Normalizes q internally."""
    q = np.asarray(q, dtype=float)
    norm = np.linalg.norm(q)
    if norm == 0.0:
        raise ValueError("zero quaternion has no rotation")
    w, x, y, z = q / norm
    return np.array([
        [1 - 2 * (y * y + z * z), 2 * (x * y - w * z),     2 * (x * z + w * y)],
        [2 * (x * y + w * z),     1 - 2 * (x * x + z * z), 2 * (y * z - w * x)],
        [2 * (x * z - w * y),     2 * (y * z + w * x),     1 - 2 * (x * x + y * y)],
    ])


def rotate_vector(q, v) -> np.ndarray:
    """Rotate 3D vector v by quaternion q (via the rotation matrix)."""
    return quat_to_rotation_matrix(q) @ np.asarray(v, dtype=float)
