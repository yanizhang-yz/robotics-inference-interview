from solution import (
    chunk,
    every_other,
    flatten_one_level,
    interleave,
    last_n,
    reverse_copy,
    rotate_left,
    sort_by_length_then_alpha,
    swap_ends,
    top_k_smallest,
)


class TestRotateLeft:
    def test_basic_rotation(self):
        assert rotate_left([1, 2, 3, 4, 5], 2) == [3, 4, 5, 1, 2]

    def test_k_exceeds_length(self):
        assert rotate_left([1, 2, 3], 7) == [2, 3, 1]  # 7 % 3 == 1

    def test_empty_list(self):
        assert rotate_left([], 3) == []

    def test_does_not_mutate_input(self):
        original = [1, 2, 3]
        rotate_left(original, 1)
        assert original == [1, 2, 3]


class TestLastN:
    def test_basic(self):
        assert last_n([1, 2, 3, 4, 5], 2) == [4, 5]

    def test_n_zero_returns_empty(self):
        assert last_n([1, 2, 3], 0) == []

    def test_n_exceeds_length_returns_all(self):
        assert last_n([1, 2], 10) == [1, 2]


class TestEveryOther:
    def test_odd_length(self):
        assert every_other([10, 20, 30, 40, 50]) == [10, 30, 50]

    def test_even_length(self):
        assert every_other(["a", "b", "c", "d"]) == ["a", "c"]

    def test_empty(self):
        assert every_other([]) == []


class TestReverseCopy:
    def test_basic(self):
        assert reverse_copy([1, 2, 3]) == [3, 2, 1]

    def test_input_unchanged(self):
        original = [1, 2, 3]
        result = reverse_copy(original)
        assert original == [1, 2, 3]
        assert result is not original

    def test_empty(self):
        assert reverse_copy([]) == []


class TestSwapEnds:
    def test_basic(self):
        assert swap_ends([1, 2, 3, 4]) == [4, 2, 3, 1]

    def test_single_element_returns_copy(self):
        original = [42]
        result = swap_ends(original)
        assert result == [42]
        assert result is not original

    def test_empty(self):
        assert swap_ends([]) == []

    def test_input_unchanged(self):
        original = [1, 2, 3]
        swap_ends(original)
        assert original == [1, 2, 3]


class TestInterleave:
    def test_equal_lengths(self):
        assert interleave([1, 3, 5], [2, 4, 6]) == [1, 2, 3, 4, 5, 6]

    def test_stops_at_shorter(self):
        assert interleave([1, 2, 3], ["a"]) == [1, "a"]

    def test_one_empty(self):
        assert interleave([], [1, 2]) == []


class TestSortByLengthThenAlpha:
    def test_length_then_alpha(self):
        words = ["banana", "kiwi", "fig", "date", "apple"]
        assert sort_by_length_then_alpha(words) == [
            "fig",
            "date",
            "kiwi",
            "apple",
            "banana",
        ]

    def test_all_same_length(self):
        assert sort_by_length_then_alpha(["bb", "aa", "cc"]) == ["aa", "bb", "cc"]

    def test_empty(self):
        assert sort_by_length_then_alpha([]) == []

    def test_input_unchanged(self):
        words = ["bb", "a"]
        sort_by_length_then_alpha(words)
        assert words == ["bb", "a"]


class TestTopKSmallest:
    def test_basic(self):
        assert top_k_smallest([5, 1, 4, 2, 3], 3) == [1, 2, 3]

    def test_k_exceeds_length(self):
        assert top_k_smallest([3, 1], 10) == [1, 3]

    def test_k_zero(self):
        assert top_k_smallest([1, 2, 3], 0) == []


class TestChunk:
    def test_even_split(self):
        assert chunk([1, 2, 3, 4], 2) == [[1, 2], [3, 4]]

    def test_ragged_last_chunk(self):
        assert chunk([1, 2, 3, 4, 5], 2) == [[1, 2], [3, 4], [5]]

    def test_size_exceeds_length(self):
        assert chunk([1, 2], 10) == [[1, 2]]

    def test_empty(self):
        assert chunk([], 3) == []


class TestFlattenOneLevel:
    def test_basic(self):
        assert flatten_one_level([[1, 2], [3], [4, 5]]) == [1, 2, 3, 4, 5]

    def test_only_one_level(self):
        # Inner nesting is preserved — this is NOT a deep flatten.
        assert flatten_one_level([[1, [2]], [3]]) == [1, [2], 3]

    def test_empty_inner_lists(self):
        assert flatten_one_level([[], [1], []]) == [1]
