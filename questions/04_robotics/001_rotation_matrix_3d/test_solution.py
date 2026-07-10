import numpy as np
import pytest
from solution import Rx, Ry, Rz, is_valid_rotation, rotate_point

TOL = 1e-9


class TestRotationMatrices:
    def test_Rz_90_rotates_x_to_y(self):
        result = rotate_point([1, 0, 0], Rz(90))
        assert np.allclose(result, [0, 1, 0], atol=1e-10)

    def test_Ry_90_rotates_x_to_neg_z(self):
        result = rotate_point([1, 0, 0], Ry(90))
        assert np.allclose(result, [0, 0, -1], atol=1e-10)

    def test_Rx_90_rotates_y_to_z(self):
        result = rotate_point([0, 1, 0], Rx(90))
        assert np.allclose(result, [0, 0, 1], atol=1e-10)

    def test_identity_at_zero(self):
        for R in [Rx(0), Ry(0), Rz(0)]:
            assert np.allclose(R, np.eye(3), atol=TOL)

    def test_360_is_identity(self):
        for R in [Rx(360), Ry(360), Rz(360)]:
            assert np.allclose(R, np.eye(3), atol=1e-10)

    def test_inverse_is_transpose(self):
        R = Rz(37) @ Ry(21) @ Rx(13)
        assert np.allclose(R.T @ R, np.eye(3), atol=1e-10)

    def test_composition_order_matters(self):
        # Rz then Rx is not the same as Rx then Rz in general
        RzRx = Rz(90) @ Rx(90)
        RxRz = Rx(90) @ Rz(90)
        assert not np.allclose(RzRx, RxRz)


class TestIsValidRotation:
    def test_valid_matrices(self):
        for R in [Rx(45), Ry(90), Rz(180)]:
            assert is_valid_rotation(R)

    def test_invalid_scaled_matrix(self):
        R = 2 * np.eye(3)
        assert not is_valid_rotation(R)

    def test_invalid_reflection(self):
        R = np.diag([-1, 1, 1])  # det = -1, not in SO(3)
        assert not is_valid_rotation(R)
