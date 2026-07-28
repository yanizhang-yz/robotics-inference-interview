import numpy as np
import pytest

from solution import (
    KVCache,
    generation_cost_with_cache,
    generation_cost_without_cache,
    kv_cache_bytes,
)


class TestKVCacheBytes:
    def test_unit_case(self):
        # 2 tensors, everything else 1.
        assert kv_cache_bytes(1, 1, 1, 1, 1, dtype_bytes=1) == 2

    def test_hand_computed_small_case(self):
        # 2 * 2 * 4 * 8 * 16 * 3 * 2 = 12288
        assert kv_cache_bytes(2, 4, 8, 16, 3, dtype_bytes=2) == 12288

    def test_llama_7b_ish_matches_readme(self):
        # 32 layers, 32 heads, head_dim 128, seq 4096, batch 8, fp16 (default).
        n = kv_cache_bytes(32, 32, 128, 4096, 8)
        assert n == 17_179_869_184
        assert n == 16 * 2**30  # exactly 16 GiB — the README's number

    def test_returns_int(self):
        assert isinstance(kv_cache_bytes(1, 2, 3, 4, 5), int)


class TestKVCacheStore:
    def test_append_until_full(self):
        cache = KVCache(max_seq_len=3)
        for i in range(3):
            assert cache.append(np.full(2, float(i)), np.full(2, float(-i))) is True
        assert len(cache) == 3
        assert cache.append(np.zeros(2), np.zeros(2)) is False
        assert len(cache) == 3  # rejected append changed nothing

    def test_full_rejection_preserves_contents(self):
        cache = KVCache(max_seq_len=1)
        cache.append(np.array([1.0, 2.0]), np.array([3.0, 4.0]))
        cache.append(np.array([9.0, 9.0]), np.array([9.0, 9.0]))  # rejected
        assert np.allclose(cache.keys, [[1.0, 2.0]])
        assert np.allclose(cache.values, [[3.0, 4.0]])

    def test_stacked_shapes_and_contents(self):
        cache = KVCache(max_seq_len=8)
        cache.append(np.array([1.0, 0.0, 0.0]), np.array([5.0, 0.0, 0.0]))
        cache.append(np.array([0.0, 1.0, 0.0]), np.array([0.0, 6.0, 0.0]))
        assert cache.keys.shape == (2, 3)
        assert cache.values.shape == (2, 3)
        assert np.allclose(cache.keys[1], [0.0, 1.0, 0.0])
        assert np.allclose(cache.values[0], [5.0, 0.0, 0.0])


class TestAttend:
    def test_matches_manual_two_step_computation(self):
        K = np.array([[1.0, 0.0], [0.0, 1.0]])
        V = np.array([[1.0, 2.0], [3.0, 4.0]])
        q = np.array([2.0, 0.0])

        cache = KVCache(max_seq_len=4)
        cache.append(K[0], V[0])
        cache.append(K[1], V[1])

        scores = K @ q / np.sqrt(2.0)                 # [sqrt(2), 0]
        w = np.exp(scores - scores.max())
        w /= w.sum()
        expected = w @ V
        assert np.allclose(cache.attend(q), expected)

    def test_orthogonal_keys_attend_to_matching_step(self):
        cache = KVCache(max_seq_len=2)
        cache.append(np.array([10.0, 0.0]), np.array([5.0, 0.0]))
        cache.append(np.array([0.0, 10.0]), np.array([0.0, 7.0]))
        # q aligned with the second key: essentially all weight on step 2.
        out = cache.attend(np.array([0.0, 10.0]))
        assert np.allclose(out, [0.0, 7.0], atol=1e-6)

    def test_output_shape_is_d(self):
        cache = KVCache(max_seq_len=4)
        cache.append(np.arange(3.0), np.arange(3.0))
        assert cache.attend(np.ones(3)).shape == (3,)


class TestGenerationCost:
    def test_with_cache_exact_formula(self):
        assert generation_cost_with_cache(1) == 1
        assert generation_cost_with_cache(5) == 5
        assert generation_cost_with_cache(100) == 100

    def test_without_cache_exact_formula(self):
        assert generation_cost_without_cache(1) == 1
        assert generation_cost_without_cache(2) == 3
        assert generation_cost_without_cache(5) == 15
        assert generation_cost_without_cache(100) == 5050

    @pytest.mark.parametrize("n", [2, 3, 10, 128])
    def test_cache_is_strictly_cheaper_from_two_tokens(self, n):
        assert generation_cost_with_cache(n) < generation_cost_without_cache(n)
