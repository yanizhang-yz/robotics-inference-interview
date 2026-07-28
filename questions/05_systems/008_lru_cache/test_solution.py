import random

import pytest

from solution import LRUCache


class _ReferenceLRU:
    """O(n)-per-op reference: list of (key, value), LRU first / MRU last."""

    def __init__(self, capacity):
        self.capacity = capacity
        self.items = []

    def get(self, key):
        for i, (k, v) in enumerate(self.items):
            if k == key:
                self.items.append(self.items.pop(i))
                return v
        return -1

    def put(self, key, value):
        for i, (k, _) in enumerate(self.items):
            if k == key:
                self.items.pop(i)
                break
        self.items.append((key, value))
        if len(self.items) > self.capacity:
            self.items.pop(0)


class TestCanonical:
    def test_leetcode_146_sequence(self):
        c = LRUCache(2)
        c.put(1, 1)
        c.put(2, 2)
        assert c.get(1) == 1
        c.put(3, 3)                 # evicts key 2
        assert c.get(2) == -1
        c.put(4, 4)                 # evicts key 1
        assert c.get(1) == -1
        assert c.get(3) == 3
        assert c.get(4) == 4

    def test_missing_key_returns_minus_one(self):
        c = LRUCache(2)
        assert c.get(99) == -1


class TestRecency:
    def test_get_refreshes_recency(self):
        c = LRUCache(2)
        c.put(1, 1)
        c.put(2, 2)
        c.get(1)                    # key 1 becomes MRU; key 2 is now LRU
        c.put(3, 3)                 # must evict 2, not 1
        assert c.get(1) == 1
        assert c.get(2) == -1
        assert c.get(3) == 3

    def test_put_existing_updates_value_and_refreshes(self):
        c = LRUCache(2)
        c.put(1, 1)
        c.put(2, 2)
        c.put(1, 100)               # update + refresh: key 2 is now LRU
        c.put(3, 3)                 # evicts 2
        assert c.get(1) == 100
        assert c.get(2) == -1
        assert c.get(3) == 3


class TestEdgeCases:
    def test_capacity_one(self):
        c = LRUCache(1)
        c.put(1, 1)
        assert c.get(1) == 1
        c.put(2, 2)                 # evicts 1
        assert c.get(1) == -1
        assert c.get(2) == 2

    def test_overwrite_does_not_grow_size(self):
        c = LRUCache(2)
        c.put(1, 1)
        c.put(1, 2)                 # overwrite: size stays 1
        c.put(2, 2)                 # size 2 — nothing may be evicted here
        assert c.get(1) == 2
        assert c.get(2) == 2

    def test_invalid_capacity(self):
        with pytest.raises(ValueError):
            LRUCache(0)


class TestRandomWorkload:
    def test_2000_ops_match_reference(self):
        rng = random.Random(0)
        capacity, key_space = 8, 16
        fast = LRUCache(capacity)
        slow = _ReferenceLRU(capacity)
        for _ in range(2000):
            key = rng.randrange(key_space)
            if rng.random() < 0.5:
                assert fast.get(key) == slow.get(key)
            else:
                value = rng.randrange(1000)
                fast.put(key, value)
                slow.put(key, value)
        # Final sweep: identical probes keep both structures in lockstep.
        for key in range(key_space):
            assert fast.get(key) == slow.get(key)
