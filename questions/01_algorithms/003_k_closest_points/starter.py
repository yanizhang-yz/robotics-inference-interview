"""
K Closest Points to Origin — YOUR ATTEMPT

Write your solution here, then run the tests against it:
    PRACTICE=1 uv run pytest questions/01_algorithms/003_k_closest_points -v

Peek at solution.py only after you've tried.
"""


def k_closest(
    points: list[tuple[float, float]], k: int
) -> list[tuple[float, float]]:
    """
    Return the k points closest to the origin (0, 0), in any order.

    Target: O(n log k) time, O(k) space.
    Hint: compare SQUARED distances x**2 + y**2 — sqrt is strictly
    increasing, so the ordering is identical and the root is wasted work.
    heapq.nsmallest(k, points, key=...) maintains the bounded heap for you.
    (Or: implement this as k_closest_to with target (0, 0).)
    """
    # TODO: implement
    raise NotImplementedError


def k_closest_to(
    points: list[tuple[float, float]], k: int, target: tuple[float, float]
) -> list[tuple[float, float]]:
    """
    Return the k points closest to `target`, in any order.

    Hint: identical to k_closest, but the squared distance is measured
    from `target`: (x - tx)**2 + (y - ty)**2.
    """
    # TODO: implement
    raise NotImplementedError
