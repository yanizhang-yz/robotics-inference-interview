import numpy as np
import pytest

from solution import causal_mask, scaled_dot_product_attention, softmax


def reference_attention(Q, K, V, mask=None):
    """Straightforward reference implementation, independent of solution.py."""
    Q, K, V = (np.asarray(a, dtype=float) for a in (Q, K, V))
    scores = Q @ K.T / np.sqrt(Q.shape[-1])
    if mask is not None:
        scores = scores + mask
    scores = scores - scores.max(axis=-1, keepdims=True)
    e = np.exp(scores)
    weights = e / e.sum(axis=-1, keepdims=True)
    return weights @ V, weights


class TestSoftmax:
    def test_large_logits_no_overflow(self):
        out = softmax(np.array([1000.0, 1001.0]))
        assert np.all(np.isfinite(out)), "large logits must not produce nan/inf"
        # Shift invariance: softmax([1000, 1001]) == softmax([0, 1]).
        assert out[1] / out[0] == pytest.approx(np.e, rel=1e-12)
        assert np.allclose(out, softmax(np.array([0.0, 1.0])))

    def test_rows_sum_to_one(self):
        rng = np.random.default_rng(0)
        x = rng.standard_normal((5, 7)) * 10
        assert np.allclose(softmax(x, axis=-1).sum(axis=-1), 1.0)

    def test_known_values(self):
        assert np.allclose(softmax(np.array([0.0, 0.0])), [0.5, 0.5])
        assert np.allclose(
            softmax(np.log(np.array([1.0, 3.0]))), [0.25, 0.75]
        )


class TestAttention:
    def test_hand_computed_case(self):
        # d_k = 4 -> sqrt(d_k) = 2; scores = [[4, 0]] / 2 = [[2, 0]].
        Q = np.array([[2.0, 0.0, 0.0, 0.0]])
        K = np.array([[2.0, 0.0, 0.0, 0.0], [0.0, 2.0, 0.0, 0.0]])
        V = np.eye(2)  # identity values: output equals the weight row
        out, w = scaled_dot_product_attention(Q, K, V)
        e2 = np.exp(2.0)
        expected = np.array([[e2 / (e2 + 1.0), 1.0 / (e2 + 1.0)]])
        assert np.allclose(w, expected)
        assert np.allclose(out, expected)

    def test_uniform_weights_when_scores_constant(self):
        rng = np.random.default_rng(1)
        Q = np.zeros((2, 3))  # Q @ K.T == 0 everywhere -> uniform rows
        K = rng.standard_normal((5, 3))
        V = rng.standard_normal((5, 4))
        out, w = scaled_dot_product_attention(Q, K, V)
        assert np.allclose(w, 1.0 / 5.0)
        assert np.allclose(out, np.tile(V.mean(axis=0), (2, 1)))

    def test_output_shapes(self):
        rng = np.random.default_rng(2)
        Q = rng.standard_normal((3, 8))
        K = rng.standard_normal((6, 8))
        V = rng.standard_normal((6, 5))
        out, w = scaled_dot_product_attention(Q, K, V)
        assert out.shape == (3, 5)
        assert w.shape == (3, 6)

    def test_attends_most_to_most_similar_key(self):
        K = np.array([[5.0, 0.0, 0.0], [0.0, 5.0, 0.0], [0.0, 0.0, 5.0]])
        Q = np.array([[5.0, 0.0, 0.0]])  # aligned with key 0
        V = np.eye(3)
        _, w = scaled_dot_product_attention(Q, K, V)
        assert int(np.argmax(w[0])) == 0
        assert w[0, 0] > w[0, 1] and w[0, 0] > w[0, 2]

    def test_none_mask_equals_zero_mask(self):
        rng = np.random.default_rng(3)
        Q = rng.standard_normal((4, 6))
        K = rng.standard_normal((5, 6))
        V = rng.standard_normal((5, 2))
        out_none, w_none = scaled_dot_product_attention(Q, K, V, mask=None)
        out_zero, w_zero = scaled_dot_product_attention(
            Q, K, V, mask=np.zeros((4, 5))
        )
        assert np.allclose(out_none, out_zero)
        assert np.allclose(w_none, w_zero)

    def test_matches_reference_on_random_inputs(self):
        rng = np.random.default_rng(42)
        Q = rng.standard_normal((7, 16))
        K = rng.standard_normal((9, 16))
        V = rng.standard_normal((9, 4))
        mask = np.where(rng.random((7, 9)) < 0.2, -np.inf, 0.0)
        mask[:, 0] = 0.0  # keep at least one position allowed per row
        out, w = scaled_dot_product_attention(Q, K, V, mask=mask)
        ref_out, ref_w = reference_attention(Q, K, V, mask=mask)
        assert np.allclose(out, ref_out)
        assert np.allclose(w, ref_w)


class TestCausalMask:
    def test_mask_values(self):
        m = causal_mask(4)
        assert m.shape == (4, 4)
        assert np.all(m[np.tril_indices(4)] == 0.0)  # allowed: j <= i
        assert np.all(np.isneginf(m[np.triu_indices(4, k=1)]))  # blocked: j > i

    def test_weights_strictly_lower_triangular(self):
        rng = np.random.default_rng(7)
        n, d = 5, 8
        Q = rng.standard_normal((n, d))
        K = rng.standard_normal((n, d))
        V = rng.standard_normal((n, 3))
        _, w = scaled_dot_product_attention(Q, K, V, mask=causal_mask(n))
        assert np.all(np.triu(w, k=1) == 0.0), "future positions must get weight 0"
        assert np.allclose(w.sum(axis=-1), 1.0)
        assert np.all(w[np.tril_indices(n)] > 0.0)

    def test_first_query_attends_only_to_first_key(self):
        rng = np.random.default_rng(8)
        Q = rng.standard_normal((3, 4))
        K = rng.standard_normal((3, 4))
        V = rng.standard_normal((3, 2))
        out, w = scaled_dot_product_attention(Q, K, V, mask=causal_mask(3))
        assert w[0, 0] == pytest.approx(1.0)
        assert np.allclose(out[0], V[0])
