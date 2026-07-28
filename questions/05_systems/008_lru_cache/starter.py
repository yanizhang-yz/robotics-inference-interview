"""
LRU Cache — YOUR ATTEMPT

Write your solution here, then run the tests against it:
    PRACTICE=1 uv run pytest questions/05_systems/008_lru_cache -v

Peek at solution.py only after you've tried.
"""


class _Node:
    """Doubly-linked-list node holding (key, value, prev, next).

    Store the KEY too, not just the value — on eviction you hold the tail
    *node* and need its key to delete the matching dict entry.
    """

    __slots__ = ("key", "value", "prev", "next")

    def __init__(self, key, value):
        # TODO: key, value, prev=None, next=None
        raise NotImplementedError


class LRUCache:
    """O(1) get/put with LRU eviction.

    Plan before coding:
    - self._map: dict key -> _Node (O(1) lookup)
    - doubly-linked list for recency order: SENTINEL head and tail nodes
      wired to each other; real nodes live between them. head.next is the
      most recently used, tail.prev the least. Sentinels mean _unlink and
      _push_front never branch on empty-list edge cases.
    - every use (get hit, put) = _unlink(node) then _push_front(node)
    """

    def __init__(self, capacity: int):
        if capacity < 1:
            raise ValueError("capacity must be >= 1")
        # TODO: capacity, map, sentinel head <-> tail
        raise NotImplementedError

    def _unlink(self, node: _Node) -> None:
        """Splice node out of the list: wire node.prev and node.next to
        each other. The node itself can keep stale pointers — _push_front
        overwrites them."""
        # TODO: implement
        raise NotImplementedError

    def _push_front(self, node: _Node) -> None:
        """Insert node right after the head sentinel (the MRU position).
        Four pointer writes: node.prev, node.next, then fix the old
        first node's prev and head's next."""
        # TODO: implement
        raise NotImplementedError

    def get(self, key):
        """Return the value or -1. A hit is a *use*: unlink + push front."""
        # TODO: implement
        raise NotImplementedError

    def put(self, key, value) -> None:
        """Insert or update; evict the LRU entry on overflow.

        Plan:
        - existing key: update node.value, refresh recency (unlink + push
          front), done — no size change.
        - new key at capacity: evict tail.prev — unlink it AND delete its
          dict entry (this is why nodes store their key).
        - insert: new node into the dict and at the front.
        """
        # TODO: implement
        raise NotImplementedError
