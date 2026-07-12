"""Reference solutions: HashMap/HashSet ceremony -> dict/set/Counter idioms.

The through-line: the patterns you know from Java collections all exist in
Python, but each one collapses to a single construct — defaultdict for
computeIfAbsent, Counter for counting maps, `in` for containsKey/contains,
dict comprehensions for entrySet loops. And unlike HashMap, every Python
dict preserves insertion order (guaranteed since 3.7).
"""

from collections import Counter, defaultdict
from typing import Any


def group_by_first_letter(words: list[str]) -> dict[str, list[str]]:
    """JAVA: computeIfAbsent(k, x -> new ArrayList<>()).add(w).
    PYTHON: defaultdict(list) materializes missing values on first access.
    """
    groups: defaultdict[str, list[str]] = defaultdict(list)
    for word in words:
        groups[word[0]].append(word)
    # Plain dict out, so callers don't inherit the auto-create behavior.
    return dict(groups)


def count_items(items: list) -> dict:
    """JAVA: HashMap + merge(item, 1, Integer::sum).
    PYTHON: Counter consumes any iterable; dict(...) strips the subclass.
    """
    return dict(Counter(items))


def top_k_frequent(items: list, k: int) -> list:
    """JAVA: count map + PriorityQueue with a comparator.
    PYTHON: most_common(k) is sorted by count, ties in first-seen order.
    """
    return [item for item, _count in Counter(items).most_common(k)]


def invert_mapping(d: dict) -> dict:
    """JAVA: loop over entrySet(), put(value, key).
    PYTHON: a dict comprehension over .items() — the entrySet loop in one line.
    """
    return {v: k for k, v in d.items()}


def merge_sum(d1: dict, d2: dict) -> dict:
    """JAVA: copy, then merge(k, v, Integer::sum) per entry.
    PYTHON: Counter.update ADDS counts (dict.update would replace them).
    Note: Counter(d1) + Counter(d2) also works but silently drops keys whose
    total is <= 0, so update() is the safer general-purpose merge.
    """
    merged = Counter(d1)
    merged.update(d2)
    return dict(merged)


def get_nested(d: dict, keys: list, default: Any = None) -> Any:
    """JAVA: nested null checks, one per level (or Optional.map chains).
    PYTHON: walk and bail early; no NPE risk because we test before we index.
    """
    current: Any = d
    for key in keys:
        if not isinstance(current, dict) or key not in current:
            return default
        current = current[key]
    return current


def first_duplicate(items: list) -> Any:
    """JAVA: HashSet with `if (!seen.add(x)) return x;`.
    PYTHON: `in` on a set is the O(1) contains(); implicit None at the end.
    """
    seen = set()
    for item in items:
        if item in seen:
            return item
        seen.add(item)
    return None


def have_common_element(a: list, b: list) -> bool:
    """JAVA: copy into HashSet + retainAll + !isEmpty.
    PYTHON: isdisjoint takes any iterable and short-circuits on first hit.
    """
    return not set(a).isdisjoint(b)


def unique_in_order(items: list) -> list:
    """JAVA: new ArrayList<>(new LinkedHashSet<>(items)).
    PYTHON: dict.fromkeys keeps first occurrences in insertion order —
    every dict is a LinkedHashMap.
    """
    return list(dict.fromkeys(items))


def first_unique_char(s: str) -> int:
    """JAVA: int[26] counts, then a second indexed for-loop.
    PYTHON: Counter + enumerate; next() with a default is the -1 sentinel.
    """
    counts = Counter(s)
    return next((i for i, c in enumerate(s) if counts[c] == 1), -1)
