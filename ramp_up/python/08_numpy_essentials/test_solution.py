import numpy as np
from numpy.testing import assert_allclose, assert_array_equal

from solution import (
    clamp_negatives,
    count_above,
    moving_average,
    normalize,
    one_hot,
    pairwise_distances,
    relu,
    row_argmax,
    softmax,
    standardize_columns,
)


class TestNormalize:
    def test_unit_length(self):
        v = np.array([3.0, 4.0])
        assert_allclose(normalize(v), [0.6, 0.8])
        assert_allclose(np.linalg.norm(normalize(v)), 1.0)

    def test_zero_vector_returns_zeros(self):
        result = normalize(np.zeros(4))
        assert_array_equal(result, np.zeros(4))
        assert not np.isnan(result).any()

    def test_does_not_mutate_input(self):
        v = np.array([3.0, 4.0])
        normalize(v)
        assert_array_equal(v, [3.0, 4.0])


class TestRelu:
    def test_mixed_signs(self):
        assert_array_equal(relu(np.array([-2.0, 0.0, 3.5])), [0.0, 0.0, 3.5])

    def test_2d_shape_preserved(self):
        x = np.array([[-1.0, 2.0], [3.0, -4.0]])
        assert_array_equal(relu(x), [[0.0, 2.0], [3.0, 0.0]])

    def test_all_negative(self):
        assert_array_equal(relu(np.array([-5.0, -0.1])), [0.0, 0.0])


class TestSoftmax:
    def test_sums_to_one(self):
        result = softmax(np.array([1.0, 2.0, 3.0]))
        assert_allclose(result.sum(), 1.0)

    def test_known_values(self):
        # Equal logits -> uniform distribution.
        assert_allclose(softmax(np.array([5.0, 5.0, 5.0, 5.0])), [0.25] * 4)

    def test_numerical_stability_large_logits(self):
        # Naive exp(1000) overflows to inf; the max-subtraction trick doesn't.
        result = softmax(np.array([1000.0, 1000.0]))
        assert not np.isnan(result).any()
        assert not np.isinf(result).any()
        assert_allclose(result, [0.5, 0.5])

    def test_shift_invariance(self):
        logits = np.array([0.5, -1.0, 2.0])
        assert_allclose(softmax(logits), softmax(logits + 100.0))


class TestOneHot:
    def test_basic(self):
        result = one_hot(np.array([0, 2, 1]), 3)
        assert_array_equal(result, [[1, 0, 0], [0, 0, 1], [0, 1, 0]])

    def test_shape(self):
        assert one_hot(np.array([4, 0]), 5).shape == (2, 5)

    def test_repeated_labels(self):
        result = one_hot(np.array([1, 1]), 2)
        assert_array_equal(result, [[0, 1], [0, 1]])

    def test_empty_labels(self):
        assert one_hot(np.array([], dtype=int), 3).shape == (0, 3)


class TestCountAbove:
    def test_basic(self):
        assert count_above(np.array([1.0, 5.0, 3.0, 7.0]), 2.5) == 3

    def test_strictly_above(self):
        assert count_above(np.array([2.0, 2.0, 3.0]), 2.0) == 1

    def test_none_above_returns_plain_int(self):
        result = count_above(np.array([1.0, 2.0]), 10.0)
        assert result == 0
        assert isinstance(result, int)


class TestClampNegatives:
    def test_basic(self):
        assert_array_equal(
            clamp_negatives(np.array([-1.0, 2.0, -3.0, 0.0])), [0.0, 2.0, 0.0, 0.0]
        )

    def test_input_not_mutated(self):
        x = np.array([-1.0, 2.0, -3.0])
        clamp_negatives(x)
        assert_array_equal(x, [-1.0, 2.0, -3.0])

    def test_all_nonnegative_unchanged(self):
        x = np.array([0.0, 1.0, 2.0])
        assert_array_equal(clamp_negatives(x), [0.0, 1.0, 2.0])


class TestRowArgmax:
    def test_basic(self):
        mat = np.array([[1, 9, 3], [7, 2, 5]])
        assert_array_equal(row_argmax(mat), [1, 0])

    def test_result_shape_is_one_per_row(self):
        mat = np.arange(12).reshape(3, 4)
        assert row_argmax(mat).shape == (3,)

    def test_tie_returns_first_index(self):
        mat = np.array([[5, 5, 1]])
        assert_array_equal(row_argmax(mat), [0])


class TestMovingAverage:
    def test_basic(self):
        assert_allclose(
            moving_average(np.array([1.0, 2.0, 3.0, 4.0]), 2), [1.5, 2.5, 3.5]
        )

    def test_valid_mode_length(self):
        result = moving_average(np.arange(10.0), 4)
        assert len(result) == 7  # len(x) - k + 1

    def test_window_equals_length(self):
        assert_allclose(moving_average(np.array([2.0, 4.0, 6.0]), 3), [4.0])


class TestPairwiseDistances:
    def test_known_distances(self):
        a = np.array([[0.0, 0.0], [1.0, 0.0]])
        b = np.array([[0.0, 3.0], [4.0, 0.0], [0.0, 0.0]])
        expected = [
            [3.0, 4.0, 0.0],
            [np.sqrt(10.0), 3.0, 1.0],
        ]
        assert_allclose(pairwise_distances(a, b), expected)

    def test_shape(self):
        a = np.zeros((4, 3))
        b = np.zeros((6, 3))
        assert pairwise_distances(a, b).shape == (4, 6)

    def test_self_distances_zero_diagonal(self):
        a = np.array([[1.0, 2.0], [3.0, 4.0]])
        d = pairwise_distances(a, a)
        assert_allclose(np.diag(d), [0.0, 0.0])
        # Distance matrix against itself is symmetric.
        assert_allclose(d, d.T)


class TestStandardizeColumns:
    def test_column_stats(self):
        mat = np.array([[1.0, 10.0], [3.0, 20.0], [5.0, 30.0]])
        result = standardize_columns(mat)
        assert_allclose(result.mean(axis=0), [0.0, 0.0], atol=1e-12)
        assert_allclose(result.std(axis=0), [1.0, 1.0])

    def test_constant_column_becomes_zeros_not_nan(self):
        mat = np.array([[7.0, 1.0], [7.0, 2.0], [7.0, 3.0]])
        result = standardize_columns(mat)
        assert not np.isnan(result).any()
        assert_array_equal(result[:, 0], [0.0, 0.0, 0.0])

    def test_input_not_mutated(self):
        mat = np.array([[1.0, 2.0], [3.0, 4.0]])
        original = mat.copy()
        standardize_columns(mat)
        assert_array_equal(mat, original)
