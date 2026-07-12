from solution import (
    caesar_shift,
    char_frequencies,
    clean_palindrome,
    find_all_indices,
    format_report,
    is_anagram,
    join_nonempty,
    longest_common_prefix,
    reverse_words,
    snake_to_camel,
)


class TestReverseWords:
    def test_basic(self):
        assert reverse_words("the quick brown fox") == "fox brown quick the"

    def test_collapses_extra_whitespace(self):
        assert reverse_words("  hello \t  world  ") == "world hello"

    def test_single_word(self):
        assert reverse_words("robot") == "robot"

    def test_whitespace_only_is_empty(self):
        assert reverse_words("   ") == ""


class TestCleanPalindrome:
    def test_classic_sentence(self):
        assert clean_palindrome("A man, a plan, a canal: Panama") is True

    def test_not_a_palindrome(self):
        assert clean_palindrome("race a car") is False

    def test_mixed_case_and_digits(self):
        assert clean_palindrome("1a2 B2A1") is True

    def test_empty_after_cleaning(self):
        # Nothing alphanumeric left -> vacuously a palindrome.
        assert clean_palindrome("!!, .") is True


class TestCharFrequencies:
    def test_basic(self):
        assert char_frequencies("banana") == {"b": 1, "a": 3, "n": 2}

    def test_case_sensitive(self):
        assert char_frequencies("aA") == {"a": 1, "A": 1}

    def test_empty_string(self):
        assert char_frequencies("") == {}


class TestCaesarShift:
    def test_basic_with_wraparound(self):
        assert caesar_shift("xyz", 3) == "abc"

    def test_preserves_case_and_punctuation(self):
        assert caesar_shift("Hello, World!", 5) == "Mjqqt, Btwqi!"

    def test_negative_shift(self):
        assert caesar_shift("cde", -2) == "abc"

    def test_full_cycle_is_identity(self):
        assert caesar_shift("Attack at dawn!", 26) == "Attack at dawn!"


class TestSnakeToCamel:
    def test_basic(self):
        assert snake_to_camel("joint_angle_limits") == "jointAngleLimits"

    def test_single_word_unchanged(self):
        assert snake_to_camel("robot") == "robot"

    def test_words_with_digits(self):
        assert snake_to_camel("arm_2_pose") == "arm2Pose"

    def test_empty_string(self):
        assert snake_to_camel("") == ""


class TestFindAllIndices:
    def test_non_overlapping(self):
        assert find_all_indices("abcabcabc", "abc") == [0, 3, 6]

    def test_overlapping(self):
        assert find_all_indices("aaaa", "aa") == [0, 1, 2]

    def test_not_found(self):
        assert find_all_indices("hello", "xyz") == []

    def test_empty_needle(self):
        assert find_all_indices("hello", "") == []


class TestLongestCommonPrefix:
    def test_basic(self):
        assert longest_common_prefix(["flower", "flow", "flight"]) == "fl"

    def test_no_common_prefix(self):
        assert longest_common_prefix(["dog", "racecar", "car"]) == ""

    def test_single_string(self):
        assert longest_common_prefix(["alone"]) == "alone"

    def test_empty_list(self):
        assert longest_common_prefix([]) == ""


class TestFormatReport:
    def test_example(self):
        assert format_report("Yani", 17, 20) == "Yani       17/20 (85.0%)"

    def test_short_name_padding(self):
        assert format_report("Al", 3, 4) == "Al".ljust(10) + " 3/4 (75.0%)"

    def test_name_longer_than_field(self):
        # 11-char name: the field expands rather than truncating.
        assert format_report("Bartholomew", 1, 2) == "Bartholomew 1/2 (50.0%)"

    def test_perfect_score(self):
        assert format_report("Bot", 5, 5) == "Bot".ljust(10) + " 5/5 (100.0%)"


class TestJoinNonempty:
    def test_no_empties(self):
        assert join_nonempty(["usr", "local", "bin"], "/") == "usr/local/bin"

    def test_skips_empty_strings(self):
        assert join_nonempty(["a", "", "b", "", ""], "-") == "a-b"

    def test_all_empty(self):
        assert join_nonempty(["", "", ""], ", ") == ""

    def test_empty_list(self):
        assert join_nonempty([], "/") == ""


class TestIsAnagram:
    def test_classic(self):
        assert is_anagram("listen", "silent") is True

    def test_different_letters(self):
        assert is_anagram("hello", "world") is False

    def test_same_letters_different_counts(self):
        # Trap for length-and-set checks: same chars, different multiplicity.
        assert is_anagram("aab", "abb") is False

    def test_both_empty(self):
        assert is_anagram("", "") is True
