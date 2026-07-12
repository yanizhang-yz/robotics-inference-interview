"""Dicts, sets, and Counter drills — write YOUR implementations in this file.

Fill in each function below, then run the tests against this file:

    PRACTICE=1 uv run pytest ramp_up/python/04_dicts_sets_counters -v

Each docstring has two lines:
    JAVA:   what you would reach for in Java
    PYTHON: the idiom to use instead
"""

from typing import Any


def group_by_first_letter(words: list[str]) -> dict[str, list[str]]:
    """Group words by their first character, preserving order within each
    group. Words are non-empty. Return a plain dict.

    group_by_first_letter(["apple", "bat", "avocado"])
        == {"a": ["apple", "avocado"], "b": ["bat"]}

    JAVA: map.computeIfAbsent(w.charAt(0), k -> new ArrayList<>()).add(w).
    PYTHON: collections.defaultdict(list) — accessing a missing key creates
        the empty list for you: groups[word[0]].append(word). Convert with
        dict(groups) at the end so the auto-create behavior doesn't leak out.
    """
    raise NotImplementedError


def count_items(items: list) -> dict:
    """Count occurrences of each item. Return a PLAIN dict (not a Counter).

    count_items(["a", "b", "a"]) == {"a": 2, "b": 1}

    JAVA: HashMap loop with merge(item, 1, Integer::sum) or getOrDefault.
    PYTHON: Counter(items) does the entire loop; wrap in dict(...) to return
        a plain dict.
    """
    raise NotImplementedError


def top_k_frequent(items: list, k: int) -> list:
    """The k most frequent items, most frequent first. Ties are broken by
    first occurrence in items. If k exceeds the number of distinct items,
    return all of them.

    top_k_frequent(["a", "a", "b", "b", "b", "c"], 2) == ["b", "a"]

    JAVA: count map, then a PriorityQueue with a comparator, then drain it.
    PYTHON: Counter(items).most_common(k) returns (item, count) pairs already
        sorted; strip the counts with a list comprehension.
    """
    raise NotImplementedError


def invert_mapping(d: dict) -> dict:
    """Swap keys and values. Values are assumed unique (and hashable).

    invert_mapping({"a": 1, "b": 2}) == {1: "a", 2: "b"}

    JAVA: loop over entrySet(), put(entry.getValue(), entry.getKey()).
    PYTHON: one dict comprehension: {v: k for k, v in d.items()}.
    """
    raise NotImplementedError


def merge_sum(d1: dict, d2: dict) -> dict:
    """Merge two str->number dicts, summing values on key collisions.
    Must NOT mutate either input. Return a plain dict.

    merge_sum({"a": 1, "b": 2}, {"b": 3, "c": 4}) == {"a": 1, "b": 5, "c": 4}

    JAVA: copy d1, then d2.forEach((k, v) -> merged.merge(k, v, Integer::sum)).
    PYTHON: merged = Counter(d1); merged.update(d2) — Counter.update ADDS
        counts instead of replacing. (Or a plain loop:
        merged[k] = merged.get(k, 0) + v.)
    """
    raise NotImplementedError


def get_nested(d: dict, keys: list, default: Any = None) -> Any:
    """Follow keys down through nested dicts; return the value at the end,
    or default if any step is missing or hits a non-dict. Empty keys -> d.

    get_nested({"a": {"b": {"c": 42}}}, ["a", "b", "c"]) == 42
    get_nested({"a": {}}, ["a", "b"], default=-1) == -1

    JAVA: a pyramid of null checks (or Optional.map chains) — one per level.
    PYTHON: walk with a for loop; `key in current` and isinstance(current,
        dict) guard each step, returning default early. d.get(k) returning
        None instead of throwing is what makes this short.
    """
    raise NotImplementedError


def first_duplicate(items: list) -> Any:
    """Scanning left to right, return the first item that has appeared
    before; None if all items are distinct. Items are hashable.

    first_duplicate([3, 1, 4, 1, 5, 9, 5]) == 1

    JAVA: HashSet with the `if (!seen.add(x)) return x;` trick.
    PYTHON: a seen set with `if item in seen: return item` — `in` on a set
        is O(1), same as contains(). Functions return None implicitly when
        they fall off the end.
    """
    raise NotImplementedError


def have_common_element(a: list, b: list) -> bool:
    """True if a and b share at least one element. Items are hashable.

    have_common_element([1, 2, 3], [3, 4]) is True

    JAVA: copy one list into a HashSet, then retainAll (mutates!) and check
        isEmpty — or loop b calling contains.
    PYTHON: not set(a).isdisjoint(b) — isdisjoint takes ANY iterable, short-
        circuits on the first hit, and mutates nothing. (set(a) & set(b)
        works too but builds the whole intersection.)
    """
    raise NotImplementedError


def unique_in_order(items: list) -> list:
    """Deduplicate while keeping the FIRST occurrence order. Items hashable.

    unique_in_order([3, 1, 3, 2, 1]) == [3, 1, 2]

    JAVA: new ArrayList<>(new LinkedHashSet<>(items)).
    PYTHON: list(dict.fromkeys(items)) — dicts preserve insertion order
        (guaranteed since 3.7), and fromkeys keeps the first occurrence of
        each key. No LinkedHashSet needed: every dict IS one.
    """
    raise NotImplementedError


def first_unique_char(s: str) -> int:
    """Index of the first character in s that appears exactly once; -1 if
    there is none.

    first_unique_char("loveleetcode") == 2

    JAVA: int[26] count array (or HashMap), then a second indexed for-loop.
    PYTHON: counts = Counter(s), then scan with enumerate(s) for the first
        index where counts[c] == 1 — next((...), -1) gives the sentinel.
    """
    raise NotImplementedError
