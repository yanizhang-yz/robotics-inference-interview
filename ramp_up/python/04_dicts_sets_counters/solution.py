"""Dicts, Sets, and Counters — reference solutions.

The through-line: every hand-rolled collection-bookkeeping loop collapses
to a single construct — defaultdict for insert-if-absent, Counter for
counting maps, `in` for membership, dict comprehensions for entry loops.
And every Python dict preserves insertion order (guaranteed since 3.7).
"""

from collections import Counter, defaultdict
from typing import Any


def group_by_first_letter(words: list[str]) -> dict[str, list[str]]:
    """defaultdict(list) materializes missing values on first access."""
    groups: defaultdict[str, list[str]] = defaultdict(list)
    for word in words:
        groups[word[0]].append(word)
    # Plain dict out, so callers don't inherit the auto-create behavior.
    return dict(groups)


def count_items(items: list) -> dict:
    """Counter consumes any iterable; dict(...) strips the subclass."""
    return dict(Counter(items))


def top_k_frequent(items: list, k: int) -> list:
    """most_common(k) is sorted by count, ties in first-seen order."""
    return [item for item, _count in Counter(items).most_common(k)]


def invert_mapping(d: dict) -> dict:
    """A dict comprehension over .items() — the entry loop in one line."""
    return {v: k for k, v in d.items()}


def merge_sum(d1: dict, d2: dict) -> dict:
    """Counter.update ADDS counts (dict.update would replace them).
    Note: Counter(d1) + Counter(d2) also works but silently drops keys whose
    total is <= 0, so update() is the safer general-purpose merge.
    """
    merged = Counter(d1)
    merged.update(d2)
    return dict(merged)


def get_nested(d: dict, keys: list, default: Any = None) -> Any:
    """Walk and bail early; no crash risk because we test before we index."""
    current: Any = d
    for key in keys:
        if not isinstance(current, dict) or key not in current:
            return default
        current = current[key]
    return current


def first_duplicate(items: list) -> Any:
    """`in` on a set is an O(1) membership test; implicit None at the end."""
    seen = set()
    for item in items:
        if item in seen:
            return item
        seen.add(item)
    return None


def have_common_element(a: list, b: list) -> bool:
    """isdisjoint takes any iterable and short-circuits on first hit."""
    return not set(a).isdisjoint(b)


def unique_in_order(items: list) -> list:
    """dict.fromkeys keeps first occurrences in insertion order —
    every dict is an insertion-ordered map.
    """
    return list(dict.fromkeys(items))


def first_unique_char(s: str) -> int:
    """Counter + enumerate; next() with a default is the -1 sentinel."""
    counts = Counter(s)
    return next((i for i, c in enumerate(s) if counts[c] == 1), -1)
