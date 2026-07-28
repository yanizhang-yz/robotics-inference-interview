import random
import statistics

import pytest
from solution import MedianStream


class TestMedianStream:
    def test_interleaved_adds_and_medians(self):
        ms = MedianStream()
        ms.add(6)
        assert ms.median() == 6
        ms.add(10)
        assert ms.median() == 8.0
        ms.add(2)
        assert ms.median() == 6
        ms.add(4)
        assert ms.median() == 5.0
        ms.add(14)
        assert ms.median() == 6

    def test_odd_count(self):
        ms = MedianStream()
        for x in [5, 1, 9]:
            ms.add(x)
        assert ms.median() == 5

    def test_even_count(self):
        ms = MedianStream()
        for x in [5, 1, 9, 3]:
            ms.add(x)
        assert ms.median() == 4.0

    def test_duplicates(self):
        ms = MedianStream()
        for _ in range(5):
            ms.add(7)
        assert ms.median() == 7
        ms.add(1)
        assert ms.median() == 7.0

    def test_all_duplicate_pairs(self):
        ms = MedianStream()
        for x in [1, 1, 2, 2]:
            ms.add(x)
        assert ms.median() == 1.5

    def test_negative_values(self):
        ms = MedianStream()
        for x in [-5, -1, -3]:
            ms.add(x)
        assert ms.median() == -3
        ms.add(-7)
        assert ms.median() == -4.0

    def test_mixed_signs(self):
        ms = MedianStream()
        ms.add(-2)
        ms.add(2)
        assert ms.median() == 0.0

    def test_ascending_insertion(self):
        ms = MedianStream()
        expected = [1, 1.5, 2, 2.5, 3, 3.5]
        for x, want in zip(range(1, 7), expected):
            ms.add(x)
            assert ms.median() == want

    def test_descending_insertion(self):
        ms = MedianStream()
        expected = [6, 5.5, 5, 4.5, 4, 3.5]
        for x, want in zip(range(6, 0, -1), expected):
            ms.add(x)
            assert ms.median() == want

    def test_empty_stream_raises(self):
        ms = MedianStream()
        with pytest.raises(ValueError):
            ms.median()

    def test_random_stream_against_statistics_median(self):
        rng = random.Random(42)
        ms = MedianStream()
        seen = []
        for i in range(1, 1001):
            x = rng.randint(-10_000, 10_000)
            ms.add(x)
            seen.append(x)
            if i % 50 == 0:
                assert ms.median() == statistics.median(seen)
