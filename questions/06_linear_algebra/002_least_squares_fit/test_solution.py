import numpy as np
import pytest

from solution import fit_line, fit_poly, predict


class TestFitLine:
    def test_exact_line_recovered(self):
        xs = np.linspace(-3, 5, 20)
        ys = 2.5 * xs - 1.25
        slope, intercept = fit_line(xs, ys)
        assert slope == pytest.approx(2.5, abs=1e-10)
        assert intercept == pytest.approx(-1.25, abs=1e-10)

    def test_noisy_line_matches_polyfit(self):
        rng = np.random.default_rng(0)
        xs = np.linspace(0, 10, 200)
        ys = 3.0 * xs + 2.0 + 0.1 * rng.standard_normal(xs.size)
        slope, intercept = fit_line(xs, ys)
        ref_slope, ref_intercept = np.polyfit(xs, ys, 1)
        assert slope == pytest.approx(ref_slope, abs=1e-8)
        assert intercept == pytest.approx(ref_intercept, abs=1e-8)
        # And close to the true generating parameters.
        assert slope == pytest.approx(3.0, abs=0.05)
        assert intercept == pytest.approx(2.0, abs=0.1)

    def test_steep_slope_still_accurate(self):
        xs = np.linspace(0.0, 1.0, 50)
        ys = 500.0 * xs + 3.0            # vertical-ish: rise of 500 over run of 1
        slope, intercept = fit_line(xs, ys)
        assert slope == pytest.approx(500.0, rel=1e-9)
        assert intercept == pytest.approx(3.0, abs=1e-6)

    def test_too_few_points_raises(self):
        with pytest.raises(ValueError):
            fit_line([1.0], [2.0])


class TestFitPoly:
    def test_exact_parabola(self):
        xs = np.linspace(-2, 2, 30)
        ys = 1.5 * xs**2 - 2.0 * xs + 0.5
        coeffs = fit_poly(xs, ys, degree=2)
        assert np.allclose(coeffs, [1.5, -2.0, 0.5], atol=1e-8)

    def test_matches_np_polyfit_on_noisy_data(self):
        rng = np.random.default_rng(42)
        xs = np.linspace(-1, 3, 100)
        ys = 0.5 * xs**3 - xs + 2.0 + 0.05 * rng.standard_normal(xs.size)
        assert np.allclose(
            fit_poly(xs, ys, degree=3), np.polyfit(xs, ys, 3), atol=1e-8
        )

    def test_underdetermined_raises(self):
        with pytest.raises(ValueError):
            fit_poly([0.0, 1.0], [1.0, 2.0], degree=2)


class TestPredict:
    def test_known_coefficients(self):
        # 2x^2 + 0x + 1 at x = [0, 1, 2] -> [1, 3, 9]
        out = predict([2.0, 0.0, 1.0], np.array([0.0, 1.0, 2.0]))
        assert np.allclose(out, [1.0, 3.0, 9.0])

    def test_fit_then_predict_round_trips(self):
        xs = np.linspace(0, 4, 25)
        ys = -1.0 * xs**2 + 3.0 * xs + 7.0
        coeffs = fit_poly(xs, ys, degree=2)
        assert np.allclose(predict(coeffs, xs), ys, atol=1e-8)

    def test_line_coeffs_work_in_predict(self):
        xs = np.array([0.0, 1.0, 2.0])
        ys = 4.0 * xs + 1.0
        slope, intercept = fit_line(xs, ys)
        assert np.allclose(predict([slope, intercept], np.array([10.0])), [41.0])
