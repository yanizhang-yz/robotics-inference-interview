"""K closest points via a bounded heap.

Comparisons use SQUARED distances: sqrt is strictly increasing on [0, inf),
so it never changes the ordering — skipping it saves a float op per point.
heapq.nsmallest keeps only k candidates while scanning the list once:
O(n log k) time, O(k) space.
"""

import heapq


def k_closest_to(
    points: list[tuple[float, float]], k: int, target: tuple[float, float]
) -> list[tuple[float, float]]:
    """The k points nearest `target`, in any order."""
    tx, ty = target
    return heapq.nsmallest(
        k, points, key=lambda p: (p[0] - tx) ** 2 + (p[1] - ty) ** 2
    )


def k_closest(
    points: list[tuple[float, float]], k: int
) -> list[tuple[float, float]]:
    """The k points nearest the origin — the target=(0, 0) special case."""
    return k_closest_to(points, k, (0.0, 0.0))
