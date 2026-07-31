import numpy as np
import pytest

from solution import (
    apply_temperature,
    greedy_sample,
    sample,
    top_k_filter,
    top_p_filter,
)


def ref_softmax(x):
    x = np.asarray(x, dtype=float)
    e = np.exp(x - x.max())
    return e / e.sum()


class TestGreedy:
    def test_picks_argmax(self):
        assert greedy_sample(np.array([0.1, 3.0, 2.9])) == 1

    def test_returns_int(self):
        assert isinstance(greedy_sample(np.array([1.0, 0.0])), int)


class TestTemperature:
    def test_low_temperature_approaches_greedy(self):
        logits = np.array([2.0, 1.0, 0.5])
        probs = ref_softmax(apply_temperature(logits, 0.01))
        assert probs[0] > 0.999
        assert np.argmax(probs) == greedy_sample(logits)

    def test_high_temperature_approaches_uniform(self):
        probs = ref_softmax(apply_temperature(np.array([2.0, 1.0, 0.5]), 100.0))
        assert np.allclose(probs, 1.0 / 3.0, atol=0.01)

    def test_unit_temperature_is_identity(self):
        logits = np.array([1.5, -2.0, 0.0])
        assert np.allclose(apply_temperature(logits, 1.0), logits)

    @pytest.mark.parametrize("t", [0.0, -1.0])
    def test_nonpositive_raises(self, t):
        with pytest.raises(ValueError):
            apply_temperature(np.array([1.0, 2.0]), t)


class TestTopK:
    def test_keeps_exactly_k_finite_entries(self):
        out = top_k_filter(np.array([3.0, 1.0, 2.5, -1.0, 0.5, 2.0]), k=3)
        assert int(np.sum(np.isfinite(out))) == 3

    def test_keeps_the_right_entries(self):
        logits = np.array([3.0, 1.0, 2.5, -1.0, 0.5, 2.0])
        out = top_k_filter(logits, k=2)
        assert set(np.flatnonzero(np.isfinite(out))) == {0, 2}
        assert out[0] == 3.0 and out[2] == 2.5
        assert out[1] == -np.inf

    def test_k_equal_to_vocab_keeps_everything(self):
        logits = np.array([1.0, 2.0, 3.0])
        assert np.all(np.isfinite(top_k_filter(logits, k=3)))

    def test_does_not_mutate_input(self):
        logits = np.array([1.0, 2.0, 3.0])
        top_k_filter(logits, k=1)
        assert np.allclose(logits, [1.0, 2.0, 3.0])

    def test_k_below_one_raises(self):
        with pytest.raises(ValueError):
            top_k_filter(np.array([1.0, 2.0]), k=0)


class TestTopP:
    # Probabilities below are exact binary fractions so cumulative sums are
    # exact and the >= p comparisons cannot wobble.

    def test_top_token_alone_exceeds_p_keeps_one(self):
        out = top_p_filter(np.array([0.5, 0.25, 0.125, 0.125]), p=0.5)
        assert np.allclose(out, [1.0, 0.0, 0.0, 0.0])

    def test_hand_built_nucleus(self):
        # cumsum sorted desc: [0.5, 0.75, 0.875, 1.0]; first >= 0.7 -> 2 tokens.
        out = top_p_filter(np.array([0.5, 0.25, 0.125, 0.125]), p=0.7)
        assert np.allclose(out, [2.0 / 3.0, 1.0 / 3.0, 0.0, 0.0])

    def test_nucleus_found_after_sorting_unsorted_input(self):
        # Same distribution, shuffled: the nucleus must follow the values.
        out = top_p_filter(np.array([0.125, 0.5, 0.125, 0.25]), p=0.7)
        assert np.allclose(out, [0.0, 2.0 / 3.0, 0.0, 1.0 / 3.0])

    def test_p_one_keeps_full_distribution(self):
        probs = np.array([0.5, 0.25, 0.25])
        assert np.allclose(top_p_filter(probs, p=1.0), probs)

    def test_renormalized_sums_to_one(self):
        out = top_p_filter(np.array([0.5, 0.25, 0.125, 0.0625, 0.0625]), p=0.6)
        assert out.sum() == pytest.approx(1.0)

    @pytest.mark.parametrize("p", [0.0, -0.1, 1.2])
    def test_out_of_range_raises(self, p):
        with pytest.raises(ValueError):
            top_p_filter(np.array([0.5, 0.5]), p=p)


class TestSample:
    LOGITS = np.array([3.0, 1.0, 2.5, -1.0, 0.5, 2.0])

    def test_seeded_rng_is_reproducible(self):
        rng1 = np.random.default_rng(123)
        rng2 = np.random.default_rng(123)
        seq1 = [sample(self.LOGITS, temperature=1.5, rng=rng1) for _ in range(20)]
        seq2 = [sample(self.LOGITS, temperature=1.5, rng=rng2) for _ in range(20)]
        assert seq1 == seq2

    def test_default_rng_is_deterministic(self):
        assert sample(self.LOGITS) == sample(self.LOGITS)

    def test_top_k_only_returns_allowed_ids(self):
        rng = np.random.default_rng(7)
        draws = {sample(self.LOGITS, top_k=2, rng=rng) for _ in range(500)}
        assert draws == {0, 2}  # the two highest logits; seeded, so exact

    def test_top_p_only_returns_allowed_ids(self):
        # softmax(LOGITS) sorted desc cumsum: 0.4525, 0.7269, ... -> nucleus {0, 2}.
        rng = np.random.default_rng(11)
        draws = {sample(self.LOGITS, top_p=0.5, rng=rng) for _ in range(500)}
        assert draws == {0, 2}

    def test_tiny_temperature_behaves_like_greedy(self):
        rng = np.random.default_rng(3)
        draws = {sample(self.LOGITS, temperature=0.01, rng=rng) for _ in range(100)}
        assert draws == {greedy_sample(self.LOGITS)}

    def test_invalid_configs_raise(self):
        with pytest.raises(ValueError):
            sample(self.LOGITS, temperature=0.0)
        with pytest.raises(ValueError):
            sample(self.LOGITS, top_k=0)
        with pytest.raises(ValueError):
            sample(self.LOGITS, top_p=1.5)
