import random
from collections import Counter

from solution import k_closest, k_closest_to


def sq_dist(p, target=(0.0, 0.0)):
    return (p[0] - target[0]) ** 2 + (p[1] - target[1]) ** 2


def as_sorted_tuples(points):
    """Order-insensitive comparison form."""
    return sorted(map(tuple, points))


class TestKClosest:
    def test_basic_exact_set(self):
        result = k_closest([(1, 3), (-2, 2), (5, 8), (0, 1)], 2)
        assert as_sorted_tuples(result) == [(-2, 2), (0, 1)]

    def test_k_zero(self):
        assert k_closest([(1, 1), (2, 2)], 0) == []

    def test_k_equals_len(self):
        pts = [(3, 3), (5, -1), (-2, 4)]
        assert as_sorted_tuples(k_closest(pts, 3)) == as_sorted_tuples(pts)

    def test_single_point(self):
        assert as_sorted_tuples(k_closest([(7, -7)], 1)) == [(7, -7)]

    def test_ties_any_valid_answer(self):
        # Four points all at distance 1 — any two of them is a correct answer.
        pts = [(1, 0), (0, 1), (-1, 0), (0, -1), (3, 3)]
        result = k_closest(pts, 2)
        assert len(result) == 2
        # Every returned point is an input point (with multiplicity respected)
        assert not Counter(map(tuple, result)) - Counter(map(tuple, pts))
        # The returned distance multiset matches an optimal answer's
        best = sorted(sq_dist(p) for p in pts)[:2]
        assert sorted(sq_dist(p) for p in result) == best

    def test_negative_coordinates(self):
        result = k_closest([(-1, -1), (-5, -5), (2, 2), (-1, 2)], 2)
        assert as_sorted_tuples(result) == [(-1, -1), (-1, 2)]

    def test_float_coordinates(self):
        result = k_closest([(0.5, 0.5), (1.5, 0.0), (0.0, 2.5)], 2)
        assert as_sorted_tuples(result) == [(0.5, 0.5), (1.5, 0.0)]

    def test_target_offset(self):
        pts = [(0, 0), (10, 10), (9, 9), (5, 5)]
        result = k_closest_to(pts, 2, (10, 10))
        assert as_sorted_tuples(result) == [(9, 9), (10, 10)]

    def test_target_offset_negative_target(self):
        pts = [(-3, 4), (0, 0), (-2, 3), (6, -8)]
        result = k_closest_to(pts, 1, (-3.0, 4.0))
        assert as_sorted_tuples(result) == [(-3, 4)]

    def test_random_cross_check_brute_force(self):
        rng = random.Random(7)
        pts = [(rng.uniform(-100, 100), rng.uniform(-100, 100)) for _ in range(500)]
        for k in (1, 10, 250, 500):
            result = k_closest(pts, k)
            brute = sorted(pts, key=sq_dist)[:k]
            assert len(result) == k
            assert sorted(map(sq_dist, result)) == sorted(map(sq_dist, brute))
            assert not Counter(map(tuple, result)) - Counter(map(tuple, pts))

    def test_random_target_cross_check_brute_force(self):
        rng = random.Random(99)
        pts = [(rng.uniform(-100, 100), rng.uniform(-100, 100)) for _ in range(500)]
        target = (12.5, -7.5)
        result = k_closest_to(pts, 40, target)
        brute = sorted(pts, key=lambda p: sq_dist(p, target))[:40]
        assert len(result) == 40
        assert sorted(sq_dist(p, target) for p in result) == sorted(
            sq_dist(p, target) for p in brute
        )
        assert not Counter(map(tuple, result)) - Counter(map(tuple, pts))
