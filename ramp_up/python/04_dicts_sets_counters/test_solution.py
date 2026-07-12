from solution import (
    count_items,
    first_duplicate,
    first_unique_char,
    get_nested,
    group_by_first_letter,
    have_common_element,
    invert_mapping,
    merge_sum,
    top_k_frequent,
    unique_in_order,
)


class TestGroupByFirstLetter:
    def test_basic_grouping(self):
        assert group_by_first_letter(["apple", "bat", "avocado", "cherry"]) == {
            "a": ["apple", "avocado"],
            "b": ["bat"],
            "c": ["cherry"],
        }

    def test_preserves_order_within_group(self):
        assert group_by_first_letter(["bat", "ball", "bird"]) == {
            "b": ["bat", "ball", "bird"]
        }

    def test_empty_list(self):
        assert group_by_first_letter([]) == {}

    def test_returns_plain_dict(self):
        # A leaked defaultdict would silently grow on missing-key access.
        result = group_by_first_letter(["apple"])
        assert type(result) is dict


class TestCountItems:
    def test_basic(self):
        assert count_items(["a", "b", "a", "c", "a", "c"]) == {"a": 3, "b": 1, "c": 2}

    def test_empty_list(self):
        assert count_items([]) == {}

    def test_returns_plain_dict(self):
        assert type(count_items(["x", "x"])) is dict


class TestTopKFrequent:
    def test_basic(self):
        assert top_k_frequent(["a", "a", "b", "b", "b", "c"], 2) == ["b", "a"]

    def test_k_one(self):
        assert top_k_frequent([1, 2, 2, 3, 2], 1) == [2]

    def test_k_larger_than_distinct_items(self):
        assert top_k_frequent(["a", "a", "b"], 10) == ["a", "b"]

    def test_ties_broken_by_first_occurrence(self):
        assert top_k_frequent(["y", "x", "y", "x"], 2) == ["y", "x"]


class TestInvertMapping:
    def test_basic(self):
        assert invert_mapping({"a": 1, "b": 2, "c": 3}) == {1: "a", 2: "b", 3: "c"}

    def test_empty(self):
        assert invert_mapping({}) == {}

    def test_double_inversion_roundtrips(self):
        d = {"x": 10, "y": 20}
        assert invert_mapping(invert_mapping(d)) == d


class TestMergeSum:
    def test_overlapping_keys_sum(self):
        assert merge_sum({"a": 1, "b": 2}, {"b": 3, "c": 4}) == {
            "a": 1,
            "b": 5,
            "c": 4,
        }

    def test_disjoint_keys(self):
        assert merge_sum({"a": 1}, {"b": 2}) == {"a": 1, "b": 2}

    def test_empty_inputs(self):
        assert merge_sum({}, {"a": 5}) == {"a": 5}
        assert merge_sum({}, {}) == {}

    def test_does_not_mutate_inputs(self):
        d1 = {"a": 1}
        d2 = {"a": 2}
        merge_sum(d1, d2)
        assert d1 == {"a": 1}
        assert d2 == {"a": 2}


class TestGetNested:
    def test_deep_hit(self):
        d = {"a": {"b": {"c": 42}}}
        assert get_nested(d, ["a", "b", "c"]) == 42

    def test_missing_key_returns_none_by_default(self):
        assert get_nested({"a": {"b": 1}}, ["a", "x"]) is None

    def test_custom_default(self):
        assert get_nested({"a": {}}, ["a", "b"], default=-1) == -1

    def test_path_through_non_dict(self):
        # "a" resolves to 5; you can't descend further.
        assert get_nested({"a": 5}, ["a", "b"]) is None


class TestFirstDuplicate:
    def test_basic(self):
        assert first_duplicate([3, 1, 4, 1, 5, 9, 5]) == 1

    def test_all_distinct(self):
        assert first_duplicate([1, 2, 3]) is None

    def test_strings(self):
        assert first_duplicate(["a", "b", "b", "a"]) == "b"

    def test_empty_list(self):
        assert first_duplicate([]) is None


class TestHaveCommonElement:
    def test_shared_element(self):
        assert have_common_element([1, 2, 3], [3, 4, 5]) is True

    def test_no_shared_element(self):
        assert have_common_element([1, 2], [3, 4]) is False

    def test_empty_inputs(self):
        assert have_common_element([], [1, 2]) is False
        assert have_common_element([], []) is False


class TestUniqueInOrder:
    def test_basic(self):
        assert unique_in_order([3, 1, 3, 2, 1]) == [3, 1, 2]

    def test_characters(self):
        assert unique_in_order(list("mississippi")) == ["m", "i", "s", "p"]

    def test_already_unique_keeps_order(self):
        assert unique_in_order([5, 4, 3]) == [5, 4, 3]

    def test_empty_list(self):
        assert unique_in_order([]) == []


class TestFirstUniqueChar:
    def test_first_char_unique(self):
        assert first_unique_char("leetcode") == 0

    def test_unique_char_in_middle(self):
        assert first_unique_char("loveleetcode") == 2

    def test_no_unique_char(self):
        assert first_unique_char("aabb") == -1

    def test_empty_string(self):
        assert first_unique_char("") == -1
