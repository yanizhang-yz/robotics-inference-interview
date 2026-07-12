from itertools import count, islice

from solution import (
    all_increasing,
    count_in_range,
    fibonacci,
    insertion_index,
    k_largest,
    merge_sorted_lists,
    min_by_distance,
    numbered_lines,
    pair_scores,
    take,
)


class TestNumberedLines:
    def test_default_start(self):
        assert numbered_lines(["alpha", "beta"]) == ["1: alpha", "2: beta"]

    def test_custom_start(self):
        assert numbered_lines(["x", "y", "z"], start=10) == ["10: x", "11: y", "12: z"]

    def test_empty(self):
        assert numbered_lines([]) == []


class TestPairScores:
    def test_basic(self):
        assert pair_scores(["ann", "bob"], [3, 7]) == {"ann": 3, "bob": 7}

    def test_unequal_lengths_stop_at_shorter(self):
        assert pair_scores(["ann", "bob", "cal"], [3, 7]) == {"ann": 3, "bob": 7}

    def test_empty(self):
        assert pair_scores([], []) == {}


class TestKLargest:
    def test_basic_sorted_descending(self):
        assert k_largest([5, 1, 9, 3, 7], 3) == [9, 7, 5]

    def test_duplicates(self):
        assert k_largest([4, 4, 1, 4], 2) == [4, 4]

    def test_k_exceeds_length(self):
        assert k_largest([2, 8], 10) == [8, 2]


class TestMergeSortedLists:
    def test_basic(self):
        assert merge_sorted_lists([[1, 4, 7], [2, 5], [3, 6, 8]]) == [
            1, 2, 3, 4, 5, 6, 7, 8,
        ]

    def test_with_empty_sublists(self):
        assert merge_sorted_lists([[], [1, 3], []]) == [1, 3]

    def test_no_lists(self):
        assert merge_sorted_lists([]) == []


class TestInsertionIndex:
    def test_between_values(self):
        assert insertion_index([10, 20, 30], 25) == 2

    def test_existing_value_returns_leftmost(self):
        assert insertion_index([1, 3, 3, 3, 5], 3) == 1

    def test_beyond_ends(self):
        assert insertion_index([1, 2, 3], 0) == 0
        assert insertion_index([1, 2, 3], 99) == 3

    def test_empty(self):
        assert insertion_index([], 5) == 0


class TestCountInRange:
    def test_inclusive_bounds(self):
        assert count_in_range([1, 2, 3, 4, 5], 2, 4) == 3

    def test_duplicates(self):
        assert count_in_range([1, 2, 2, 2, 3], 2, 2) == 3

    def test_empty_window(self):
        assert count_in_range([1, 5, 9], 6, 8) == 0


class TestMinByDistance:
    def test_basic(self):
        points = [(0.0, 0.0), (3.0, 4.0), (1.0, 1.0)]
        assert min_by_distance(points, (1.0, 2.0)) == (1.0, 1.0)

    def test_exact_match(self):
        points = [(5.0, 5.0), (2.0, 2.0)]
        assert min_by_distance(points, (2.0, 2.0)) == (2.0, 2.0)

    def test_tie_returns_first(self):
        points = [(-1.0, 0.0), (1.0, 0.0)]
        assert min_by_distance(points, (0.0, 0.0)) == (-1.0, 0.0)


class TestAllIncreasing:
    def test_increasing(self):
        assert all_increasing([1, 2, 5, 9]) is True

    def test_not_strictly_increasing(self):
        assert all_increasing([1, 2, 2, 3]) is False
        assert all_increasing([3, 1, 2]) is False

    def test_empty_and_single(self):
        assert all_increasing([]) is True
        assert all_increasing([42]) is True


class TestTake:
    def test_from_list(self):
        assert take([1, 2, 3, 4], 2) == [1, 2]

    def test_from_infinite_generator(self):
        assert take(count(100), 3) == [100, 101, 102]

    def test_fewer_items_than_n(self):
        assert take([1, 2], 5) == [1, 2]

    def test_zero(self):
        assert take([1, 2, 3], 0) == []


class TestFibonacci:
    def test_first_ten_via_islice(self):
        assert list(islice(fibonacci(), 10)) == [0, 1, 1, 2, 3, 5, 8, 13, 21, 34]

    def test_is_lazy_iterator(self):
        gen = fibonacci()
        assert next(gen) == 0
        assert next(gen) == 1
        assert next(gen) == 1

    def test_independent_instances(self):
        a, b = fibonacci(), fibonacci()
        next(a)  # advance only a
        assert next(a) == 1
        assert next(b) == 0
