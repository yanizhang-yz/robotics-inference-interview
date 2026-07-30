"""Dicts, sets, and Counter drills — write YOUR implementations in this file.

Fill in each function below, then run the tests against this file:

    PRACTICE=1 uv run pytest ramp_up/python/04_dicts_sets_counters -v

Each docstring names the Python idiom to reach for.
"""

from typing import Any
from collections import Counter, defaultdict


def group_by_first_letter(words: list[str]) -> dict[str, list[str]]:
    """Group words by their first character, preserving order within each
    group. Words are non-empty. Return a plain dict.

    group_by_first_letter(["apple", "bat", "avocado"])
        == {"a": ["apple", "avocado"], "b": ["bat"]}

    HINT: collections.defaultdict(list) — accessing a missing key creates
        the empty list for you: groups[word[0]].append(word). Convert with
        dict(groups) at the end so the auto-create behavior doesn't leak out.
    """
    groups: defaultdict[str, list[str]] = defaultdict(list)
    for word in words:
        groups[word[0]].append(word)
    return dict(groups)


def count_items(items: list) -> dict:
    """Count occurrences of each item. Return a PLAIN dict (not a Counter).

    count_items(["a", "b", "a"]) == {"a": 2, "b": 1}

    HINT: Counter(items) does the entire loop; wrap in dict(...) to return
        a plain dict.
    """
    return dict(Counter(items))
    


def top_k_frequent(items: list, k: int) -> list:
    """The k most frequent items, most frequent first. Ties are broken by
    first occurrence in items. If k exceeds the number of distinct items,
    return all of them.

    top_k_frequent(["a", "a", "b", "b", "b", "c"], 2) == ["b", "a"]

    HINT: Counter(items).most_common(k) returns (item, count) pairs already
        sorted; strip the counts with a list comprehension.
    """
    return [item for item, _count in Counter(items).most_common(k)]

def invert_mapping(d: dict) -> dict:
    """Swap keys and values. Values are assumed unique (and hashable).

    invert_mapping({"a": 1, "b": 2}) == {1: "a", 2: "b"}

    HINT: one dict comprehension: {v: k for k, v in d.items()}.
    """
    return { v: k for k, v in d.items()}


def merge_sum(d1: dict, d2: dict) -> dict:
    """Merge two str->number dicts, summing values on key collisions.
    Must NOT mutate either input. Return a plain dict.

    merge_sum({"a": 1, "b": 2}, {"b": 3, "c": 4}) == {"a": 1, "b": 5, "c": 4}

    HINT: merged = Counter(d1); merged.update(d2) — Counter.update ADDS
        counts instead of replacing. (Or a plain loop:
        merged[k] = merged.get(k, 0) + v.)
    """
    merged = Counter(d1)
    merged.update(d2)
    return dict(merged)


def get_nested(d: dict, keys: list, default: Any = None) -> Any:
    """Follow keys down through nested dicts; return the value at the end,
    or default if any step is missing or hits a non-dict. Empty keys -> d.

    get_nested({"a": {"b": {"c": 42}}}, ["a", "b", "c"]) == 42
    get_nested({"a": {}}, ["a", "b"], default=-1) == -1

    HINT: walk with a for loop; `key in current` and isinstance(current,
        dict) guard each step, returning default early. d.get(k) returning
        None instead of throwing is what makes this short.
    """
    current: Any = d
    for key in keys:
        if not isinstance(current, dict) or key not in current:
            return default
        current = current[key]
    return current


def first_duplicate(items: list) -> Any:
    """Scanning left to right, return the first item that has appeared
    before; None if all items are distinct. Items are hashable.

    first_duplicate([3, 1, 4, 1, 5, 9, 5]) == 1

    HINT: a seen set with `if item in seen: return item` — `in` on a set
        is an O(1) membership test. Functions return None implicitly when
        they fall off the end.
    """
    seen = set()
    for item in items:
        if item in seen:
            return item
        seen.add(item)
    return None


def have_common_element(a: list, b: list) -> bool:
    """True if a and b share at least one element. Items are hashable.

    have_common_element([1, 2, 3], [3, 4]) is True

    HINT: not set(a).isdisjoint(b) — isdisjoint takes ANY iterable, short-
        circuits on the first hit, and mutates nothing. (set(a) & set(b)
        works too but builds the whole intersection.)
    """
    return not set(a).isdisjoint(b)


def unique_in_order(items: list) -> list:
    """Deduplicate while keeping the FIRST occurrence order. Items hashable.

    unique_in_order([3, 1, 3, 2, 1]) == [3, 1, 2]

    HINT: list(dict.fromkeys(items)) — dicts preserve insertion order
        (guaranteed since 3.7), and fromkeys keeps the first occurrence of
        each key. No insertion-ordered set type needed: every dict IS one.
    """
    return list(dict.fromkeys(items))


def first_unique_char(s: str) -> int:
    """Index of the first character in s that appears exactly once; -1 if
    there is none.

    first_unique_char("loveleetcode") == 2

    HINT: counts = Counter(s), then scan with enumerate(s) for the first
        index where counts[c] == 1 — next((...), -1) gives the sentinel.
    """
    counts = Counter(s)
    return next((i for i,c in enumerate(s) if counts[c]  == 1), -1)
