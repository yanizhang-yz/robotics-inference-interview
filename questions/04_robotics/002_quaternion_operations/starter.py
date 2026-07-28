"""
Quaternion Operations — YOUR ATTEMPT

Write your solution here, then run the tests against it:
    PRACTICE=1 uv run pytest questions/04_robotics/002_quaternion_operations -v

Peek at solution.py only after you've tried.

Convention: quaternions are (w, x, y, z) — scalar part FIRST.
"""

import numpy as np


def quat_multiply(q1, q2) -> np.ndarray:
    """
    Hamilton product q1 ⊗ q2, returned as array (w, x, y, z).

    Plan — with q1 = (w1, x1, y1, z1), q2 = (w2, x2, y2, z2):
        w = w1*w2 - x1*x2 - y1*y2 - z1*z2
        x = w1*x2 + x1*w2 + y1*z2 - z1*y2
        y = w1*y2 - x1*z2 + y1*w2 + z1*x2
        z = w1*z2 + x1*y2 - y1*x2 + z1*w2
    Order matters: q1 ⊗ q2 applies q2's rotation first, then q1's.
    """
    # TODO: implement
    raise NotImplementedError


def quat_conjugate(q) -> np.ndarray:
    """
    Conjugate (w, -x, -y, -z). For a UNIT quaternion this is the inverse:
    it undoes the rotation.
    """
    # TODO: implement
    raise NotImplementedError


def quat_to_rotation_matrix(q) -> np.ndarray:
    """
    3x3 rotation matrix for quaternion q.

    Plan:
    1. NORMALIZE q first (divide by its 4-vector norm) — callers may hand
       you a non-unit quaternion, and the formula below assumes unit norm.
    2. Standard (w, x, y, z) formula:
       R = [[1-2(y²+z²),   2(xy-wz),   2(xz+wy)],
            [  2(xy+wz), 1-2(x²+z²),   2(yz-wx)],
            [  2(xz-wy),   2(yz+wx), 1-2(x²+y²)]]
    """
    # TODO: implement
    raise NotImplementedError


def rotate_vector(q, v) -> np.ndarray:
    """
    Rotate 3D vector v by quaternion q.

    Simplest: quat_to_rotation_matrix(q) @ v. (Equivalent alternative:
    the sandwich q ⊗ (0, v) ⊗ q* using quat_multiply and quat_conjugate.)
    """
    # TODO: implement
    raise NotImplementedError
