# 07 — Iteration protocol, heapq, bisect

## What this covers

The Python replacements for a whole shelf of Java machinery: `Iterator`,
`PriorityQueue`, `TreeMap`, `Comparator`, and index-based `for` loops.
Python collapses all of that into the **iteration protocol** (anything with
`__iter__` can be looped, zipped, sliced, and summed) plus two tiny stdlib
modules: `heapq` (heap operations on a plain `list`) and `bisect` (binary
search on a sorted `list`).

## Why this trips up Java developers

- In Java, "give me each element with its index" means `for (int i = 0; ...)`.
  In Python you never manage the index yourself — `enumerate(xs, start=n)`
  hands you `(index, element)` pairs.
- Java's `PriorityQueue` is a class you construct, offer into, and poll from.
  Python's `heapq` is a set of *functions* that treat a plain `list` as a heap
  — and for the common interview cases (`nlargest`, `nsmallest`, `merge`)
  you don't even loop.
- Java's `TreeMap.ceilingKey` / `floorKey` thinking maps to `bisect_left` /
  `bisect_right` on a sorted list. Two bisects give you an O(log n) range
  count with no tree in sight.
- Java's `Comparator.comparing(...)` + `Collections.min(...)` is just
  `min(xs, key=...)` — the key function replaces the whole Comparator class.
- Generators (`yield`) are lazy iterators you'd need a hand-rolled `Iterator`
  subclass for in Java. They can be **infinite**; you take a finite slice
  with `itertools.islice`.

> **Min-heap note:** `heapq` is a MIN-heap. For max-heap behavior, negate the
> values on the way in and negate again on the way out. Java's
> `PriorityQueue` is *also* a min-heap by default (you'd pass
> `Comparator.reverseOrder()`), so the trick should feel familiar.

## Drills

| Drill | Idiom it teaches | Java equivalent |
|---|---|---|
| `numbered_lines` | `enumerate(lines, start=n)` | `for (int i = 0; i < n; i++)` with manual offset |
| `pair_scores` | `dict(zip(names, scores))` | index loop filling a `HashMap<String, Integer>` |
| `k_largest` | `heapq.nlargest(k, nums)` | `PriorityQueue` + offer/poll loop keeping size k |
| `merge_sorted_lists` | `heapq.merge(*lists)` | k-way merge with a `PriorityQueue` of (value, listIdx) |
| `insertion_index` | `bisect.bisect_left` | `Collections.binarySearch` / `TreeMap.ceilingKey` |
| `count_in_range` | `bisect_right(hi) - bisect_left(lo)` | `TreeMap.subMap(lo, true, hi, true).size()` |
| `min_by_distance` | `min(points, key=...)` | `Collections.min(points, Comparator.comparing(...))` |
| `all_increasing` | `all()` + pairwise generator expression | loop with early `return false` |
| `take` | `itertools.islice` on ANY iterable | `Iterator` + counted `while (it.hasNext())` loop |
| `fibonacci` | infinite generator with `yield` | custom `Iterator<Long>` class with mutable state |

## Run it

```bash
# against the reference solutions
uv run pytest ramp_up/python/07_iteration_heapq_bisect -v

# against your own attempt in starter.py
PRACTICE=1 uv run pytest ramp_up/python/07_iteration_heapq_bisect -v
```
