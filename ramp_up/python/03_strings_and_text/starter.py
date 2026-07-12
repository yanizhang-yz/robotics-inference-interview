"""Strings and text drills — write YOUR implementations in this file.

Fill in each function below, then run the tests against this file:

    PRACTICE=1 uv run pytest ramp_up/python/03_strings_and_text -v

Each docstring has two lines:
    JAVA:   what you would reach for in Java
    PYTHON: the idiom to use instead
"""


def reverse_words(s: str) -> str:
    """Reverse the order of words in s, collapsing extra whitespace.

    reverse_words("  the quick   brown fox ") == "fox brown quick the"

    JAVA: s.trim().split("\\s+"), Collections.reverse, then String.join(" ", ...).
    PYTHON: s.split() with no args already trims and collapses whitespace;
        reverse the list with [::-1]; stitch back with " ".join(...).
    """
    raise NotImplementedError


def clean_palindrome(s: str) -> bool:
    """True if s is a palindrome considering only alphanumeric characters,
    ignoring case. Empty (after cleaning) counts as a palindrome.

    clean_palindrome("A man, a plan, a canal: Panama") is True

    JAVA: two pointers from both ends, skipping non-letters with
        Character.isLetterOrDigit and lowering with Character.toLowerCase.
    PYTHON: build the cleaned list with a comprehension
        ([c.casefold() for c in s if c.isalnum()]) and compare it to its
        [::-1] reverse. casefold() is the aggressive, correct lowercase.
    """
    raise NotImplementedError


def char_frequencies(s: str) -> dict[str, int]:
    """Map each character in s to how many times it appears (case-sensitive).

    char_frequencies("banana") == {"b": 1, "a": 3, "n": 2}

    JAVA: HashMap<Character, Integer> with map.merge(c, 1, Integer::sum)
        or getOrDefault + put.
    PYTHON: collections.Counter(s) does the whole loop; wrap in dict(...) to
        return a plain dict. (Manual version: d[c] = d.get(c, 0) + 1.)
    """
    raise NotImplementedError


def caesar_shift(s: str, k: int) -> str:
    """Shift each ASCII letter by k positions (wrapping around the alphabet),
    preserving case. Non-letters pass through unchanged. k may be negative.

    caesar_shift("Hello, World!", 5) == "Mjqqt, Btwqi!"

    JAVA: loop with charAt, cast to int, shift, watch out that Java's % can
        return NEGATIVE values, cast back to char, append to StringBuilder.
    PYTHON: ord(c) / chr(n) instead of casts; Python's % is always
        non-negative so (ord(c) - base + k) % 26 just works; build with
        "".join(generator) instead of StringBuilder.
    """
    raise NotImplementedError


def snake_to_camel(s: str) -> str:
    """Convert snake_case to camelCase; the first word stays lowercase.

    snake_to_camel("joint_angle_limits") == "jointAngleLimits"
    snake_to_camel("robot") == "robot"

    JAVA: StringBuilder loop, uppercasing w.charAt(0) of every word after
        the first and appending substring(1).
    PYTHON: first, *rest = s.split("_")  (star-unpacking!), then
        first + "".join(word.capitalize() for word in rest).
    """
    raise NotImplementedError


def find_all_indices(s: str, sub: str) -> list[int]:
    """All indices where sub occurs in s, including overlapping occurrences,
    in increasing order. If sub is empty, return [].

    find_all_indices("aaaa", "aa") == [0, 1, 2]

    JAVA: while loop with s.indexOf(sub, fromIndex), advancing fromIndex.
    PYTHON: same shape but with s.find(sub, start) — find returns -1 instead
        of throwing; advance start to i + 1 to catch overlaps. No regex needed.
    """
    raise NotImplementedError


def longest_common_prefix(strs: list[str]) -> str:
    """Longest prefix shared by every string in strs. Empty list -> "".

    longest_common_prefix(["flower", "flow", "flight"]) == "fl"

    JAVA: take strs[0], shrink it in a loop while some string doesn't
        startWith it — index bookkeeping everywhere.
    PYTHON: zip(*strs) transposes the strings into columns of characters
        (stopping at the shortest); walk columns while len(set(chars)) == 1.
    """
    raise NotImplementedError


def format_report(name: str, score: int, total: int) -> str:
    """One report line: name left-aligned in a 10-char field, then a space,
    then "score/total", then the percentage in parentheses with 1 decimal.
    total is always > 0.

    format_report("Yani", 17, 20) == "Yani       17/20 (85.0%)"
    (i.e. "Yani".ljust(10) + " 17/20 (85.0%)")

    JAVA: String.format("%-10s %d/%d (%.1f%%)", name, score, total, pct).
    PYTHON: one f-string with inline format specs:
        :<10 left-aligns in 10 chars, and :.1% multiplies by 100, keeps
        1 decimal, and appends % — so pass the raw ratio score / total.
    """
    raise NotImplementedError


def join_nonempty(parts: list[str], sep: str) -> str:
    """Join only the non-empty strings in parts with sep.

    join_nonempty(["usr", "", "local", "", "bin"], "/") == "usr/local/bin"

    JAVA: StringBuilder loop with an if(!p.isEmpty()) guard and trailing-
        separator bookkeeping (or streams + Collectors.joining).
    PYTHON: sep.join(p for p in parts if p) — join takes any iterable, and
        empty strings are falsy so `if p` filters them.
    """
    raise NotImplementedError


def is_anagram(a: str, b: str) -> bool:
    """True if a and b contain exactly the same characters with the same
    counts (case-sensitive).

    is_anagram("listen", "silent") is True

    JAVA: toCharArray(), Arrays.sort both, Arrays.equals — or a count array.
    PYTHON: sorted(a) == sorted(b), or better: Counter(a) == Counter(b)
        (O(n), and dict equality ignores insertion order).
    """
    raise NotImplementedError
