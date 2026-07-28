import math

import numpy as np
import pytest
from solution import quat_conjugate, quat_multiply, quat_to_rotation_matrix, rotate_vector

IDENTITY_Q = (1.0, 0.0, 0.0, 0.0)


def axis_angle_quat(axis, angle_deg):
    """Unit quaternion (w, x, y, z) for a rotation of angle_deg about a unit axis."""
    axis = np.asarray(axis, dtype=float)
    axis = axis / np.linalg.norm(axis)
    half = math.radians(angle_deg) / 2.0
    return np.array([math.cos(half), *(math.sin(half) * axis)])


# Same elementary-rotation convention as questions/04_robotics/001_rotation_matrix_3d:
def Rx(angle_deg):
    t = math.radians(angle_deg)
    c, s = math.cos(t), math.sin(t)
    return np.array([[1, 0, 0], [0, c, -s], [0, s, c]], dtype=float)


def Ry(angle_deg):
    t = math.radians(angle_deg)
    c, s = math.cos(t), math.sin(t)
    return np.array([[c, 0, s], [0, 1, 0], [-s, 0, c]], dtype=float)


def Rz(angle_deg):
    t = math.radians(angle_deg)
    c, s = math.cos(t), math.sin(t)
    return np.array([[c, -s, 0], [s, c, 0], [0, 0, 1]], dtype=float)


def random_unit_quat(rng):
    q = rng.standard_normal(4)
    return q / np.linalg.norm(q)


def assert_is_valid_rotation(R):
    np.testing.assert_allclose(R.T @ R, np.eye(3), atol=1e-10)
    assert np.linalg.det(R) == pytest.approx(1.0, abs=1e-10)


class TestQuatToRotationMatrix:
    def test_identity_quaternion_gives_identity_matrix(self):
        np.testing.assert_allclose(quat_to_rotation_matrix(IDENTITY_Q), np.eye(3), atol=1e-12)

    def test_90_about_x_matches_known_matrix(self):
        R = quat_to_rotation_matrix(axis_angle_quat([1, 0, 0], 90))
        expected = np.array([[1, 0, 0], [0, 0, -1], [0, 1, 0]], dtype=float)
        np.testing.assert_allclose(R, expected, atol=1e-10)

    def test_90_about_y_matches_known_matrix(self):
        R = quat_to_rotation_matrix(axis_angle_quat([0, 1, 0], 90))
        expected = np.array([[0, 0, 1], [0, 1, 0], [-1, 0, 0]], dtype=float)
        np.testing.assert_allclose(R, expected, atol=1e-10)

    def test_90_about_z_matches_known_matrix(self):
        R = quat_to_rotation_matrix(axis_angle_quat([0, 0, 1], 90))
        expected = np.array([[0, -1, 0], [1, 0, 0], [0, 0, 1]], dtype=float)
        np.testing.assert_allclose(R, expected, atol=1e-10)

    @pytest.mark.parametrize("angle_deg", [-135, -30, 17, 45, 90, 210])
    def test_axis_rotations_match_001_convention(self, angle_deg):
        # Cross-check against the Rx/Ry/Rz convention of question 001_rotation_matrix_3d.
        for axis, R_ref in [([1, 0, 0], Rx), ([0, 1, 0], Ry), ([0, 0, 1], Rz)]:
            R = quat_to_rotation_matrix(axis_angle_quat(axis, angle_deg))
            np.testing.assert_allclose(R, R_ref(angle_deg), atol=1e-10)

    def test_output_is_valid_rotation_for_random_quats(self):
        rng = np.random.default_rng(0)
        for _ in range(20):
            assert_is_valid_rotation(quat_to_rotation_matrix(random_unit_quat(rng)))

    def test_non_normalized_input_is_normalized_internally(self):
        q = axis_angle_quat([0, 0, 1], 90)
        np.testing.assert_allclose(
            quat_to_rotation_matrix(3.7 * q), quat_to_rotation_matrix(q), atol=1e-10
        )
        assert_is_valid_rotation(quat_to_rotation_matrix(3.7 * q))


class TestQuatMultiply:
    def test_identity_is_neutral_element(self):
        rng = np.random.default_rng(1)
        q = random_unit_quat(rng)
        np.testing.assert_allclose(quat_multiply(q, IDENTITY_Q), q, atol=1e-12)
        np.testing.assert_allclose(quat_multiply(IDENTITY_Q, q), q, atol=1e-12)

    def test_composition_consistent_with_matrix_product(self):
        # R(q1 ⊗ q2) must equal R(q1) @ R(q2).
        rng = np.random.default_rng(2)
        for _ in range(20):
            q1, q2 = random_unit_quat(rng), random_unit_quat(rng)
            np.testing.assert_allclose(
                quat_to_rotation_matrix(quat_multiply(q1, q2)),
                quat_to_rotation_matrix(q1) @ quat_to_rotation_matrix(q2),
                atol=1e-10,
            )

    def test_two_quarter_turns_make_a_half_turn(self):
        q90 = axis_angle_quat([0, 0, 1], 90)
        np.testing.assert_allclose(
            quat_to_rotation_matrix(quat_multiply(q90, q90)),
            quat_to_rotation_matrix(axis_angle_quat([0, 0, 1], 180)),
            atol=1e-10,
        )


class TestQuatConjugate:
    def test_unit_quat_times_conjugate_is_identity(self):
        rng = np.random.default_rng(3)
        q = random_unit_quat(rng)
        np.testing.assert_allclose(quat_multiply(q, quat_conjugate(q)), IDENTITY_Q, atol=1e-10)

    def test_conjugate_inverts_rotation(self):
        rng = np.random.default_rng(4)
        for _ in range(10):
            q = random_unit_quat(rng)
            v = rng.standard_normal(3)
            np.testing.assert_allclose(
                rotate_vector(quat_conjugate(q), rotate_vector(q, v)), v, atol=1e-9
            )


class TestRotateVector:
    def test_matches_matrix_times_vector(self):
        rng = np.random.default_rng(5)
        for _ in range(10):
            q = random_unit_quat(rng)
            v = rng.standard_normal(3)
            np.testing.assert_allclose(
                rotate_vector(q, v), quat_to_rotation_matrix(q) @ v, atol=1e-10
            )

    def test_90_about_z_rotates_x_to_y(self):
        result = rotate_vector(axis_angle_quat([0, 0, 1], 90), [1.0, 0.0, 0.0])
        np.testing.assert_allclose(result, [0.0, 1.0, 0.0], atol=1e-10)

    def test_preserves_length(self):
        rng = np.random.default_rng(6)
        q = random_unit_quat(rng)
        v = rng.standard_normal(3)
        assert np.linalg.norm(rotate_vector(q, v)) == pytest.approx(np.linalg.norm(v), abs=1e-10)
