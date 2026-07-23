"""Thread-safe fixed-capacity ring buffer.

The lock covers every read-modify-write; fullness is computed inline from
_size (never by calling the public locked methods) to avoid self-deadlock
on the non-reentrant Lock.
"""

import threading


class RingBuffer:
    def __init__(self, capacity: int):
        if capacity < 1:
            raise ValueError("capacity must be >= 1")
        self._buf = [None] * capacity
        self._capacity = capacity
        self._head = 0          # index of the oldest item
        self._size = 0
        self._lock = threading.Lock()

    def push(self, item) -> bool:
        """Append at the tail. Return False (and do nothing) when full."""
        with self._lock:
            if self._size == self._capacity:
                return False
            self._buf[(self._head + self._size) % self._capacity] = item
            self._size += 1
            return True

    def push_overwrite(self, item) -> None:
        """Append at the tail; when full, evict the oldest item."""
        with self._lock:
            if self._size == self._capacity:
                # Tail and head coincide when full: write over the oldest slot
                # and advance head — eviction and insertion in one step.
                self._buf[self._head] = item
                self._head = (self._head + 1) % self._capacity
            else:
                self._buf[(self._head + self._size) % self._capacity] = item
                self._size += 1

    def pop(self):
        """Remove and return the oldest item, or None when empty."""
        with self._lock:
            if self._size == 0:
                return None
            item = self._buf[self._head]
            self._buf[self._head] = None          # drop the reference
            self._head = (self._head + 1) % self._capacity
            self._size -= 1
            return item

    def __len__(self) -> int:
        with self._lock:
            return self._size

    def is_empty(self) -> bool:
        with self._lock:
            return self._size == 0

    def is_full(self) -> bool:
        with self._lock:
            return self._size == self._capacity
