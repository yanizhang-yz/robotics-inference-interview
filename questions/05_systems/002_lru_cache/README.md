# LRU Cache

**Difficulty:** Medium  
**Source:** One of the most-reported interview questions in autonomous-vehicle loops: Zoox candidates report implementing an LRU cache framed as caching computed trajectory paths; Waymo loops reportedly probe the doubly-linked-list + hash-map design; Cruise variants reportedly swap LRU for LFU. Also LeetCode 146, a top-frequency question industry-wide  
**Tags:** `hash-map`, `doubly-linked-list`, `cache`, `eviction`, `O(1)`

## Problem

Design a fixed-capacity **least-recently-used cache** where both operations run in
O(1):

- `LRUCache(capacity)` — positive fixed capacity.
- `get(key)` — return the value, or `-1` if the key is absent.
- `put(key, value)` — insert or update; if inserting would exceed capacity, evict
  the least-recently-used entry first.

Both `get` and `put` count as a "use" of the key.

```
c = LRUCache(2)
c.put(1, 1); c.put(2, 2)
c.get(1)        # -> 1   (key 1 is now most recently used)
c.put(3, 3)     # evicts key 2, the LRU
c.get(2)        # -> -1
```

## Why this appears in robotics inference interviews

Inference services cache aggressively — feature tensors keyed by sensor frame,
compiled engines keyed by input shape, tokenized prompts keyed by string — and
always under bounded memory, where LRU is the default eviction policy. The
question tests whether you can compose two data structures to buy O(1) for both
halves of the job: a hash map gives O(1) lookup but no ordering; a list gives
ordering but O(n) lookup; neither alone suffices.

## Approach

The classic hand-rolled design — learn this one first:

- `dict` mapping `key -> node`, plus a **doubly-linked list** of nodes holding
  `(key, value)`. Most-recently-used at the head, least-recently-used at the tail.
- **Sentinel head and tail nodes** wired to each other at construction: real nodes
  live between them, so `_unlink` / `_push_front` never branch on empty-list or
  end-of-list edge cases.
- Each node stores its **key** as well as its value — on eviction you hold the tail
  *node* and need its key to delete the dict entry.
- `get`: dict hit → `_unlink(node)`, `_push_front(node)`, return value.
- `put` on an existing key: update the value **and** refresh recency the same way.
- `put` on a new key at capacity: evict `tail.prev` (unlink it and delete its dict
  entry), then insert the new node at the front and into the dict.

**Time:** O(1) for both `get` and `put`. **Space:** O(capacity).

The Python-fluency shortcut: `collections.OrderedDict` with `move_to_end(key)` and
`popitem(last=False)` gives the same behavior in a few lines. Worth mentioning in
an interview — but an interviewer may ask for the hand-rolled version explicitly,
so `solution.py` implements the hand-rolled one.

## Follow-ups to be ready for

1. **LFU** (the reported Cruise variant): evict the *least-frequently*-used key —
   a dict of `frequency -> doubly-linked list` plus a min-frequency pointer keeps
   both operations O(1).
2. **Thread safety:** one lock around each whole operation is correct; discuss
   granularity — a global lock serializes everything, sharding the cache into
   independently locked stripes reduces contention.
3. **TTL entries:** store an expiry timestamp per node and treat expired hits as
   misses (lazy expiry), or run active expiry off a heap of deadlines.
4. **Why `OrderedDict` works:** it is itself a hash map + doubly-linked list
   underneath — `move_to_end` is exactly the unlink + push-front you hand-rolled.
