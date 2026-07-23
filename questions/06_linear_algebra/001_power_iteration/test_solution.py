import numpy as np
import pytest

from solution import power_iteration, rayleigh_quotient


def direction_matches(v, expected, abs_tol=1e-6):
    """Eigenvectors are defined up to sign: compare |cos angle| to 1."""
    v = v / np.linalg.norm(v)
    expected = expected / np.linalg.norm(expected)
    return abs(float(v @ expected)) == pytest.approx(1.0, abs=abs_tol)


class TestRayleighQuotient:
    def test_exact_for_eigenvector(self):
        A = np.diag([3.0, 1.0])
        assert rayleigh_quotient(A, np.array([1.0, 0.0])) == pytest.approx(3.0)

    def test_scale_invariant(self):
        A = np.array([[2.0, 1.0], [1.0, 2.0]])
        v = np.array([1.0, 2.0])
        assert rayleigh_quotient(A, v) == pytest.approx(rayleigh_quotient(A, 10 * v))

    def test_zero_vector_raises(self):
        with pytest.raises(ValueError):
            rayleigh_quotient(np.eye(2), np.zeros(2))


class TestPowerIteration:
    def test_diagonal_matrix(self):
        val, vec = power_iteration(np.diag([2.0, 1.0]))
        assert val == pytest.approx(2.0, abs=1e-8)
        assert direction_matches(vec, np.array([1.0, 0.0]))

    def test_symmetric_matches_numpy(self):
        rng = np.random.default_rng(42)
        X = rng.standard_normal((50, 4))
        A = X.T @ X                       # covariance-style symmetric PSD
        val, vec = power_iteration(A)
        ref_vals, ref_vecs = np.linalg.eigh(A)
        assert val == pytest.approx(ref_vals[-1], rel=1e-6)
        assert direction_matches(vec, ref_vecs[:, -1])

    def test_negative_dominant_eigenvalue_converges(self):
        val, vec = power_iteration(np.diag([-5.0, 2.0]))
        assert val == pytest.approx(-5.0, abs=1e-6)
        assert direction_matches(vec, np.array([1.0, 0.0]))

    def test_unit_length_output(self):
        _, vec = power_iteration(np.array([[4.0, 1.0], [1.0, 3.0]]))
        assert np.linalg.norm(vec) == pytest.approx(1.0, abs=1e-9)

    def test_zero_matrix_null_space(self):
        val, vec = power_iteration(np.zeros((3, 3)))
        assert val == pytest.approx(0.0)
        assert np.linalg.norm(vec) == pytest.approx(1.0, abs=1e-9)

    def test_non_square_raises(self):
        with pytest.raises(ValueError):
            power_iteration(np.ones((2, 3)))

    def test_pca_first_component(self):
        # Points stretched along y = x: the principal direction is [1, 1]/sqrt(2).
        rng = np.random.default_rng(7)
        t = rng.standard_normal(200)
        pts = np.stack([t, t + 0.05 * rng.standard_normal(200)], axis=1)
        cov = pts.T @ pts / len(pts)
        _, vec = power_iteration(cov)
        # Noisy statistical data: the principal direction is near [1, 1],
        # not exactly it — tolerance reflects the 0.05 noise level.
        assert direction_matches(vec, np.array([1.0, 1.0]), abs_tol=1e-2)
