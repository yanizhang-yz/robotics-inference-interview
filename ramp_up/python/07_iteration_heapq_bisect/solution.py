"""
Iteration protocol, heapq, bisect — reference solutions.

Java's Iterator / PriorityQueue / TreeMap / Comparator machinery collapses
into the iteration protocol plus two stdlib modules that operate on plain
lists. Note: heapq is a MIN-heap (so is Java's PriorityQueue by default);
negate values for max-heap behavior.
"""

import heapq
from bisect import bisect_left, bisect_right
from collections.abc import Iterable, Iterator
from itertools import islice


def numbered_lines(lines: list[str], start: int = 1) -> list[str]:
    # enumerate hands out (number, line) pairs — no manual index variable.
    return [f"{n}: {line}" for n, line in enumerate(lines, start=start)]


def pair_scores(names: list[str], scores: list[int]) -> dict[str, int]:
    # zip pairs elements and stops at the shorter input — no length checks.
    return dict(zip(names, scores))


def k_largest(nums: list[int], k: int) -> list[int]:
    # Replaces the whole PriorityQueue offer/poll loop; result is sorted
    # descending. (For k close to len(nums), sorted(nums, reverse=True)[:k]
    # is fine too.)
    return heapq.nlargest(k, nums)


def merge_sorted_lists(lists: list[list[int]]) -> list[int]:
    # heapq.merge is a lazy k-way merge — the same (value, listIndex)
    # PriorityQueue dance you'd hand-write in Java, prebuilt.
    return list(heapq.merge(*lists))


def insertion_index(sorted_nums: list[int], x: int) -> int:
    # bisect_left: leftmost slot where x keeps the list sorted. This is the
    # TreeMap.ceilingKey mental model, but positional.
    return bisect_left(sorted_nums, x)


def count_in_range(sorted_nums: list[int], lo: int, hi: int) -> int:
    # Two binary searches bracket the inclusive [lo, hi] window: O(log n).
    return bisect_right(sorted_nums, hi) - bisect_left(sorted_nums, lo)


def min_by_distance(
    points: list[tuple[float, float]], target: tuple[float, float]
) -> tuple[float, float]:
    tx, ty = target
    # key= replaces Comparator.comparingDouble. Squared distance preserves
    # ordering, so skip the sqrt.
    return min(points, key=lambda p: (p[0] - tx) ** 2 + (p[1] - ty) ** 2)


def all_increasing(nums: list[int]) -> bool:
    # zip(nums, nums[1:]) yields adjacent pairs; all() short-circuits on the
    # first violation, just like an early `return false` in a Java loop.
    return all(a < b for a, b in zip(nums, nums[1:]))


def take(iterable: Iterable, n: int) -> list:
    # islice works on ANY iterable — including infinite generators, which
    # list(iterable)[:n] would hang on.
    return list(islice(iterable, n))


def fibonacci() -> Iterator[int]:
    # A generator function IS the Iterator class: `yield` suspends here and
    # resumes on the next next() call. Infinite on purpose — consumers slice.
    a, b = 0, 1
    while True:
        yield a
        a, b = b, a + b
