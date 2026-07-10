import pytest
from solution import sliding_window_maximum


class TestSlidingWindowMaximum:
    def test_basic(self):
        assert sliding_window_maximum([1, 3, -1, -3, 5, 3, 6, 7], 3) == [3, 3, 5, 5, 6, 7]

    def test_window_size_one(self):
        assert sliding_window_maximum([4, 2, 7, 1], 1) == [4, 2, 7, 1]

    def test_window_equals_array(self):
        assert sliding_window_maximum([3, 1, 2], 3) == [3]

    def test_all_same(self):
        assert sliding_window_maximum([5, 5, 5, 5], 2) == [5, 5, 5]

    def test_decreasing(self):
        assert sliding_window_maximum([9, 7, 5, 3, 1], 2) == [9, 7, 5, 3]

    def test_increasing(self):
        assert sliding_window_maximum([1, 3, 5, 7, 9], 3) == [5, 7, 9]

    def test_empty(self):
        assert sliding_window_maximum([], 3) == []

    def test_single_element(self):
        assert sliding_window_maximum([42], 1) == [42]

    def test_negatives(self):
        assert sliding_window_maximum([-5, -3, -1, -4, -2], 2) == [-3, -1, -1, -2]
