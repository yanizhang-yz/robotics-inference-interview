"""
Thread-Safe Ring Buffer — YOUR ATTEMPT

Write your solution here, then run the tests against it:
    PRACTICE=1 uv run pytest questions/05_systems/001_thread_safe_ring_buffer -v

Peek at solution.py only after you've tried.
"""

import threading


class RingBuffer:
    """Fixed-capacity circular queue; all operations O(1) and thread-safe.

    Plan before coding:
    - storage: a fixed list of `capacity` slots
    - `head` = index of the oldest item, `size` = current count
      (size is how you tell full from empty — both have head == tail otherwise)
    - tail slot to write into is (head + size) % capacity
    - ONE threading.Lock, `with self._lock:` in every public method;
      compute fullness inline from size — calling your own locked is_full()
      from inside push() deadlocks a non-reentrant Lock.
    """

    def __init__(self, capacity: int):
        if capacity < 1:
            raise ValueError("capacity must be >= 1")
        # TODO: buffer, head, size, lock
        raise NotImplementedError

    def push(self, item) -> bool:
        """Append at the tail. Return False (and do nothing) when full."""
        # TODO: implement
        raise NotImplementedError

    def push_overwrite(self, item) -> None:
        """Append at the tail; when full, evict the OLDEST item.

        Hint: when full, the tail slot IS the head slot — write there and
        advance head. Eviction and insertion are one step.
        """
        # TODO: implement
        raise NotImplementedError

    def pop(self):
        """Remove and return the oldest item, or None when empty."""
        # TODO: implement
        raise NotImplementedError

    def __len__(self) -> int:
        # TODO: implement
        raise NotImplementedError

    def is_empty(self) -> bool:
        # TODO: implement
        raise NotImplementedError

    def is_full(self) -> bool:
        # TODO: implement
        raise NotImplementedError
