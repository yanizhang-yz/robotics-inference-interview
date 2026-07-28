"""
Median of a Data Stream — YOUR ATTEMPT

Write your solution here, then run the tests against it:
    PRACTICE=1 uv run pytest questions/01_algorithms/002_median_of_data_stream -v

Peek at solution.py only after you've tried.
"""


class MedianStream:
    """Running median of a stream: add() in O(log n), median() in O(1).

    Plan before coding:
    - TWO heaps that split the stream around the median:
        * `lower` = max-heap of the smaller half (top = largest small value)
        * `upper` = min-heap of the larger half (top = smallest large value)
    - heapq is min-only, so store NEGATED values in `lower`
      (push -x, read -lower[0]).
    - After every add, rebalance so sizes differ by at most 1; convention:
      `lower` keeps the extra element when the total count is odd.
    - median(): odd count -> top of `lower`; even count -> average of the
      two tops; empty stream -> raise ValueError.
    """

    def __init__(self) -> None:
        # TODO: the two heaps
        raise NotImplementedError

    def add(self, x: float) -> None:
        """Insert x: push onto the correct half, then rebalance the sizes."""
        # TODO: implement
        raise NotImplementedError

    def median(self) -> float:
        """Return the current median. Raise ValueError if the stream is empty."""
        # TODO: implement
        raise NotImplementedError
