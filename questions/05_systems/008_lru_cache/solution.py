"""LRU cache: hash map for O(1) lookup + doubly-linked list for O(1) recency.

Sentinel head/tail nodes mean _unlink and _push_front never branch on
empty-list edge cases; head.next is the most recently used node, tail.prev
the least. Each node stores its key so eviction can delete the dict entry.

Python-fluency shortcut: collections.OrderedDict (itself a hash map +
doubly-linked list) gives the same behavior via move_to_end(key) and
popitem(last=False) — but interviewers often ask for the hand-rolled
version explicitly, so that is what this file implements.
"""


class _Node:
    __slots__ = ("key", "value", "prev", "next")

    def __init__(self, key, value):
        self.key = key          # kept so eviction can delete the dict entry
        self.value = value
        self.prev = None
        self.next = None


class LRUCache:
    def __init__(self, capacity: int):
        if capacity < 1:
            raise ValueError("capacity must be >= 1")
        self._capacity = capacity
        self._map = {}                      # key -> _Node
        self._head = _Node(None, None)      # sentinel: head.next = MRU
        self._tail = _Node(None, None)      # sentinel: tail.prev = LRU
        self._head.next = self._tail
        self._tail.prev = self._head

    def _unlink(self, node: _Node) -> None:
        """Splice node out of the list."""
        node.prev.next = node.next
        node.next.prev = node.prev

    def _push_front(self, node: _Node) -> None:
        """Insert node right after the head sentinel (the MRU position)."""
        node.prev = self._head
        node.next = self._head.next
        self._head.next.prev = node
        self._head.next = node

    def get(self, key):
        """Return the value or -1. A hit counts as a use."""
        node = self._map.get(key)
        if node is None:
            return -1
        self._unlink(node)
        self._push_front(node)
        return node.value

    def put(self, key, value) -> None:
        """Insert or update; evict the least-recently-used entry on overflow."""
        node = self._map.get(key)
        if node is not None:
            node.value = value
            self._unlink(node)
            self._push_front(node)
            return
        if len(self._map) == self._capacity:
            lru = self._tail.prev
            self._unlink(lru)
            del self._map[lru.key]
        node = _Node(key, value)
        self._map[key] = node
        self._push_front(node)
