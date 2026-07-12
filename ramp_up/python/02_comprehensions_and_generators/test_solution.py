from solution import (
    adjacent_pairs,
    all_products,
    countdown,
    first_matching,
    flatten_matrix,
    running_totals,
    squares_of_evens,
    transpose,
    unique_first_letters,
    word_length_map,
)


class TestSquaresOfEvens:
    def test_mixed(self):
        assert squares_of_evens([1, 2, 3, 4, 5]) == [4, 16]

    def test_no_evens(self):
        assert squares_of_evens([1, 3, 5]) == []

    def test_empty(self):
        assert squares_of_evens([]) == []

    def test_negatives(self):
        assert squares_of_evens([-2, -1, 0]) == [4, 0]


class TestWordLengthMap:
    def test_basic(self):
        assert word_length_map(["hi", "world"]) == {"hi": 2, "world": 5}

    def test_empty_list(self):
        assert word_length_map([]) == {}

    def test_empty_string_key(self):
        assert word_length_map([""]) == {"": 0}


class TestUniqueFirstLetters:
    def test_dedupes_and_lowercases(self):
        assert unique_first_letters(["Apple", "avocado", "Banana"]) == {"a", "b"}

    def test_ignores_empty_strings(self):
        assert unique_first_letters(["", "cat", ""]) == {"c"}

    def test_all_empty(self):
        assert unique_first_letters(["", ""]) == set()


class TestTranspose:
    def test_square(self):
        assert transpose([[1, 2], [3, 4]]) == [[1, 3], [2, 4]]

    def test_rectangular(self):
        assert transpose([[1, 2, 3], [4, 5, 6]]) == [[1, 4], [2, 5], [3, 6]]

    def test_single_row(self):
        assert transpose([[1, 2, 3]]) == [[1], [2], [3]]

    def test_round_trip(self):
        m = [[1, 2, 3], [4, 5, 6]]
        assert transpose(transpose(m)) == m


class TestFlattenMatrix:
    def test_row_major_order(self):
        assert flatten_matrix([[1, 2], [3, 4], [5]]) == [1, 2, 3, 4, 5]

    def test_empty_matrix(self):
        assert flatten_matrix([]) == []

    def test_empty_rows(self):
        assert flatten_matrix([[], [1], []]) == [1]


class TestRunningTotals:
    def test_basic(self):
        assert running_totals([1, 2, 3, 4]) == [1, 3, 6, 10]

    def test_with_negatives(self):
        assert running_totals([5, -2, -3]) == [5, 3, 0]

    def test_empty(self):
        assert running_totals([]) == []


class TestCountdown:
    def test_yields_n_down_to_one(self):
        assert list(countdown(4)) == [4, 3, 2, 1]

    def test_is_lazy_iterator(self):
        gen = countdown(3)
        assert next(gen) == 3
        assert next(gen) == 2
        # The rest is still pending — generators produce on demand.
        assert list(gen) == [1]

    def test_zero_yields_nothing(self):
        assert list(countdown(0)) == []

    def test_single_use(self):
        gen = countdown(2)
        assert list(gen) == [2, 1]
        assert list(gen) == []  # exhausted — unlike a re-streamable Java List


class TestFirstMatching:
    def test_finds_first(self):
        assert first_matching([1, 3, 4, 6], lambda x: x % 2 == 0) == 4

    def test_default_when_no_match(self):
        assert first_matching([1, 3], lambda x: x > 10, default=-1) == -1

    def test_default_defaults_to_none(self):
        assert first_matching([], lambda x: True) is None

    def test_short_circuits(self):
        seen = []

        def spy(x):
            seen.append(x)
            return x == 2

        assert first_matching([1, 2, 3, 4], spy) == 2
        assert seen == [1, 2]  # never looked at 3 or 4


class TestAdjacentPairs:
    def test_basic(self):
        assert adjacent_pairs([1, 2, 3]) == [(1, 2), (2, 3)]

    def test_two_elements(self):
        assert adjacent_pairs(["a", "b"]) == [("a", "b")]

    def test_short_lists_give_empty(self):
        assert adjacent_pairs([1]) == []
        assert adjacent_pairs([]) == []


class TestAllProducts:
    def test_basic(self):
        assert all_products(["red", "blue"], ["S", "M"]) == [
            ("red", "S"),
            ("red", "M"),
            ("blue", "S"),
            ("blue", "M"),
        ]

    def test_one_empty_gives_empty(self):
        assert all_products([], ["S", "M"]) == []
        assert all_products(["red"], []) == []

    def test_count(self):
        assert len(all_products([1, 2, 3], "ab")) == 6
