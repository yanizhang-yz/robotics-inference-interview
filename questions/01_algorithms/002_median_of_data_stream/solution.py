"""Running median via two heaps.

`_lower` is a max-heap of the smaller half (values stored negated — heapq is
min-only), `_upper` is a min-heap of the larger half. Invariants after every
add(): every value in _lower <= every value in _upper, and
len(_lower) == len(_upper) or len(_lower) == len(_upper) + 1.
"""

import heapq


class MedianStream:
    def __init__(self) -> None:
        self._lower: list[float] = []  # negated: -_lower[0] is the half's max
        self._upper: list[float] = []  # plain:    _upper[0] is the half's min

    def add(self, x: float) -> None:
        """O(log n): push onto the correct half, then rebalance the sizes."""
        if not self._lower or x <= -self._lower[0]:
            heapq.heappush(self._lower, -x)
        else:
            heapq.heappush(self._upper, x)

        # Rebalance so |lower| - |upper| is 0 or 1
        if len(self._lower) > len(self._upper) + 1:
            heapq.heappush(self._upper, -heapq.heappop(self._lower))
        elif len(self._upper) > len(self._lower):
            heapq.heappush(self._lower, -heapq.heappop(self._upper))

    def median(self) -> float:
        """O(1): read the heap tops. Raises ValueError on an empty stream."""
        if not self._lower:
            raise ValueError("median() of an empty stream")
        if len(self._lower) > len(self._upper):
            return -self._lower[0]
        return (-self._lower[0] + self._upper[0]) / 2
