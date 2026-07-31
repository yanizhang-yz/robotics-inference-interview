import numpy as np
import pytest
from solution import KalmanFilter1D


def assert_symmetric_positive_definite(P):
    np.testing.assert_allclose(P, P.T, atol=1e-9)
    assert np.linalg.eigvalsh(P).min() > 0


class TestInitialState:
    def test_constructor_seeds_state_and_covariance(self):
        kf = KalmanFilter1D(
            process_var=0.1, measurement_var=1.0,
            initial_pos=3.0, initial_vel=-1.5, initial_uncertainty=500.0,
        )
        assert kf.position == pytest.approx(3.0)
        assert kf.velocity == pytest.approx(-1.5)
        np.testing.assert_allclose(kf.covariance, 500.0 * np.eye(2))


class TestConvergence:
    def test_repeated_identical_measurements_converge(self):
        # Tiny process noise + the same measurement over and over: the estimate
        # must settle on the measurement, and the post-update position variance
        # must shrink monotonically.
        kf = KalmanFilter1D(process_var=1e-8, measurement_var=1.0)
        variances = []
        for _ in range(50):
            kf.predict(dt=1.0)
            kf.update(5.0)
            variances.append(kf.covariance[0, 0])
        assert kf.position == pytest.approx(5.0, abs=1e-3)
        assert all(b <= a + 1e-12 for a, b in zip(variances, variances[1:]))
        assert variances[-1] < variances[0]

    def test_tracks_constant_velocity_and_beats_raw_measurements(self):
        rng = np.random.default_rng(42)
        true_pos, true_vel, dt, sigma = 2.0, 1.5, 0.1, 0.5
        kf = KalmanFilter1D(process_var=0.01, measurement_var=sigma**2)
        kf_errors, raw_errors = [], []
        for step in range(200):
            true_pos += true_vel * dt
            z = true_pos + sigma * rng.standard_normal()
            kf.predict(dt)
            kf.update(z)
            if step >= 100:  # score the second half, after convergence
                kf_errors.append(kf.position - true_pos)
                raw_errors.append(z - true_pos)
        assert kf.velocity == pytest.approx(true_vel, abs=0.2)
        kf_rmse = float(np.sqrt(np.mean(np.square(kf_errors))))
        raw_rmse = float(np.sqrt(np.mean(np.square(raw_errors))))
        assert kf_rmse < raw_rmse  # smoothing must beat the raw sensor


class TestUncertaintyRhythm:
    def test_predict_only_grows_uncertainty(self):
        # Converge first, then simulate missed detections: covariance must grow
        # every predict-only step.
        kf = KalmanFilter1D(process_var=0.5, measurement_var=1.0)
        for _ in range(20):
            kf.predict(dt=0.1)
            kf.update(1.0)
        variances = [kf.covariance[0, 0]]
        for _ in range(5):
            kf.predict(dt=0.1)
            variances.append(kf.covariance[0, 0])
        assert all(b > a for a, b in zip(variances, variances[1:]))

    def test_update_shrinks_uncertainty(self):
        kf = KalmanFilter1D(process_var=0.5, measurement_var=1.0)
        kf.predict(dt=0.1)
        before = kf.covariance[0, 0]
        kf.update(0.0)
        assert kf.covariance[0, 0] < before


class TestKalmanGain:
    def test_huge_measurement_var_barely_moves_estimate(self):
        # A sensor we don't trust: even a wild measurement shifts the state
        # only microscopically.
        kf = KalmanFilter1D(
            process_var=0.1, measurement_var=1e9, initial_uncertainty=1.0
        )
        kf.predict(dt=1.0)
        kf.update(100.0)
        assert abs(kf.position) < 1e-4

    def test_tiny_measurement_var_snaps_to_measurement(self):
        # A near-perfect sensor and a clueless prior: the estimate jumps
        # straight to the measurement.
        kf = KalmanFilter1D(
            process_var=0.1, measurement_var=1e-9, initial_uncertainty=1000.0
        )
        kf.update(42.0)
        assert kf.position == pytest.approx(42.0, abs=1e-6)


class TestNumericalHealth:
    def test_covariance_stays_symmetric_positive_definite(self):
        rng = np.random.default_rng(0)
        kf = KalmanFilter1D(process_var=0.5, measurement_var=1.0)
        pos = 0.0
        for _ in range(100):
            pos += 0.3
            kf.predict(dt=0.1)
            kf.update(pos + 0.5 * rng.standard_normal())
            assert_symmetric_positive_definite(kf.covariance)
