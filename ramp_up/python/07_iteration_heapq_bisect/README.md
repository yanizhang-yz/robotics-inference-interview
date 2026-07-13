# 07 — Iteration, heapq, bisect: retiring `Iterator`, `PriorityQueue`, and `TreeMap`

After this lesson you can loop over anything without managing an index, write a lazy
infinite stream in four lines, keep a running top-k without building a `PriorityQueue`
class, and answer "how many timestamps fall in this window?" in O(log n) with two binary
searches. This is the single highest-density interview topic in the Python track: heaps
and binary search appear in a huge fraction of medium-difficulty problems, and Python
compresses each one to a one-liner — if you know the one-liner.

## The Java you know

Everything below is machinery you have written many times. Keep it in mind as the anchor;
the whole lesson is "here is what each block collapses into."

```java
// 1. Top-3 largest: keep a size-3 min-heap, evict the smallest.
PriorityQueue<Integer> heap = new PriorityQueue<>();   // min-heap by default
for (int x : nums) {
    heap.offer(x);
    if (heap.size() > 3) heap.poll();                  // evict current minimum
}

// 2. Number each line: manage the index yourself.
for (int i = 0; i < lines.size(); i++) {
    out.add((i + 1) + ": " + lines.get(i));
}

// 3. Closest point: build a Comparator object to define the ordering.
Point best = Collections.min(points,
        Comparator.comparingDouble(p -> distance(p, target)));

// 4. Sorted lookups: a TreeMap (a balanced tree) for floor/ceiling queries.
TreeMap<Long, Pose> log = new TreeMap<>();
Long t = log.ceilingKey(stamp);                        // smallest key >= stamp
```

Python replaces all four with: the **iteration protocol** (one rule that makes every
container loopable), `enumerate`/`zip` (index bookkeeping done for you), `heapq` (heap
*functions* that operate on a plain `list`), and `bisect` (binary search on a sorted
`list`). No new container classes anywhere — you keep using `list`.

## The lesson

### Iterables and iterators: what `for` actually does

Two words you must own, because every builtin in this lesson is described with them:

- An **iterable** is any object that can hand out its elements one at a time — lists,
  strings, dicts, files, and generators all qualify. Formally: anything you can pass
  to the builtin `iter()`.
- An **iterator** is the cursor object that `iter()` returns. It remembers the current
  position and gives you the next element each time you call the builtin `next()` on it.
  This is exactly Java's `Iterator<T>`, except `hasNext()` doesn't exist — instead,
  `next()` past the end raises a `StopIteration` exception, and the `for` loop catches
  it silently as its stop signal.

```python
it = iter([10, 20, 30])   # ask the list for a cursor
next(it)                  # -> 10
next(it)                  # -> 20
next(it)                  # -> 30
next(it)                  # -> raises StopIteration
```

A Python `for` loop is literally sugar for that: call `iter()` once, call `next()` until
`StopIteration`. The consequence is huge: **any function built on the protocol works on
any iterable** — `sum`, `max`, `min`, `sorted`, `list`, `zip`, `enumerate`,
`heapq.merge`, `itertools.islice` all accept a list, a file, or an infinite stream
without caring which. So `sum(x * x for x in range(4))` returns `14` with no list ever
built, and `list(iter("abc"))` is `['a', 'b', 'c']` — strings are iterables too.

**Gotcha:** iterators are single-use. Once a cursor hits the end it stays exhausted —
looping over it a second time yields nothing, with no error:

```python
it = iter([1, 2])
list(it)   # -> [1, 2]
list(it)   # -> []       already drained — no exception, just empty
```

If you need two passes, call `list()` on it first to copy the elements into a real list.
(More lazy-view traps: Gotcha 13 in [`../LEARNING_POINTS.md`](../LEARNING_POINTS.md).)

### `enumerate` and `zip`: the death of the index loop

In Python, writing `for i in range(len(xs))` just to read `xs[i]` marks you as a Java
developer in the first minute of an interview. The two replacements:

**`enumerate(xs, start=n)`** yields `(index, element)` pairs (`start` defaults to 0) —
it deletes block 2 of the Java anchor:

```python
# Each (index, element) pair lands straight into two variables `n, line` in the loop header
[f"{n}: {line}" for n, line in enumerate(["a", "b"], start=1)]
# -> ['1: a', '2: b']
```

**`zip(xs, ys)`** walks two (or more) iterables in lockstep, yielding tuples — the
parallel-arrays loop from Java, without the loop:

```python
list(zip([1, 2, 3], [10, 20]))                 # -> [(1, 10), (2, 20)]
dict(zip(["ann", "bob", "cal"], [3, 7]))        # -> {'ann': 3, 'bob': 7}
```

**Gotcha (both examples above show it):** `zip` silently stops at the *shorter* input —
the `3` and `"cal"` just vanished. Java would have thrown `IndexOutOfBoundsException`;
Python drops the tail with no warning. When mismatched lengths mean a bug in your data,
pass `strict=True` (Python 3.10+) to make `zip` raise `ValueError` instead.

### Generators: `yield` writes the Iterator class for you

A **generator function** is a function containing the `yield` keyword. Calling it runs
*no code at all* — it returns a **generator** (an iterator). Each `next()` call runs the
body until it hits `yield`, hands that value out, and **freezes the function mid-line**:
all locals keep their values, and the next `next()` resumes right after the `yield`.
That frozen state is what your Java `Iterator<Long>` class stored in fields.

```java
// Java: a whole class to stream Fibonacci numbers
class Fib implements Iterator<Long> {
    private long a = 0, b = 1;
    public boolean hasNext() { return true; }
    public Long next() { long r = a; long t = a + b; a = b; b = t; return r; }
}
```

```python
def fibonacci():
    a, b = 0, 1
    while True:          # infinite ON PURPOSE — nobody runs this to completion
        yield a          # emit, then freeze here until next() is called again
        a, b = b, a + b

g = fibonacci()
next(g), next(g), next(g)   # -> (0, 1, 1)
```

This is **lazy evaluation**: values are computed only when asked for, so an infinite
stream costs nothing until consumed. To take a finite prefix, use
`itertools.islice(iterable, n)` — "slice" for iterators (plain `[:n]` slicing only works
on sequences like lists):

```python
from itertools import islice
list(islice(fibonacci(), 10))   # -> [0, 1, 1, 2, 3, 5, 8, 13, 21, 34]
```

**Gotcha:** never write `list(gen)[:n]` to take a prefix — `list()` tries to drain the
whole generator first, and on an infinite one that loops forever; `islice` stops pulling
after `n` items. Also, each `fibonacci()` call creates an independent generator with its
own frozen state — advancing one does not move the others.

### `heapq`: a priority queue that is just functions on a list

The data structure, in plain English: a **heap** (here, a binary *min-heap*) arranges
values in a plain array so the smallest is always at index 0, and restoring that
arrangement after an insert or removal costs only O(log n). That "smallest on top" rule
is the **heap property**; a **priority queue** (always serve the smallest item first) is
the abstract idea a heap implements. Java wraps this in a class. Python gives you
*functions* that maintain the heap property inside a `list` you own:

```java
// Java                                  // Python
PriorityQueue<Integer> pq =              import heapq
    new PriorityQueue<>();               h = [5, 1, 9, 3, 7]
pq.offer(5); /* ... */                   heapq.heapify(h)      # rearrange in place, O(n)
pq.peek();   // smallest                 h[0]                  # peek: -> 1
pq.poll();   // remove smallest          heapq.heappop(h)      # -> 1
pq.offer(0);                             heapq.heappush(h, 0)
```

After `heapify`, `h` prints as `[1, 3, 9, 5, 7]` — **not sorted**, and that's correct: a
heap only guarantees the minimum sits at index 0. Never iterate a heap expecting sorted
order; pop repeatedly instead.

Both languages default to a **MIN-heap**. For max-heap behavior Java passes
`Comparator.reverseOrder()`; Python's trick is to negate values on the way in and negate
again on the way out:

```python
h = [-x for x in [5, 1, 9]]
heapq.heapify(h)
-heapq.heappop(h)   # -> 9
```

Two more things Java's `PriorityQueue` makes you build by hand, prebuilt here:

```python
heapq.nlargest(3, [5, 1, 9, 3, 7])            # -> [9, 7, 5]  (sorted descending!)
heapq.nsmallest(2, [5, 1, 9, 3, 7])           # -> [1, 3]
list(heapq.merge([1, 4, 7], [2, 5], [3, 6, 8]))  # -> [1, 2, 3, 4, 5, 6, 7, 8]
```

`nlargest(k, xs)` is the entire "keep a size-k heap, evict the minimum" loop from the
Java anchor, in one call, result already sorted. `merge` is the lazy k-way merge of
already-sorted inputs. And because tuples compare element-by-element (next section), you
can push `(priority, payload)` tuples and the heap orders by priority:

```python
h = []
heapq.heappush(h, (2, "b")); heapq.heappush(h, (1, "a"))
heapq.heappop(h)   # -> (1, 'a')
```

### `bisect`: binary search you never have to write again

**Binary search** finds a position in a *sorted* list by repeatedly halving the search
range — O(log n) instead of scanning. You have written it in Java (and probably had the
`mid` off-by-one bug at least once). Python ships it, framed as a question with no
off-by-ones: *"at which index would `x` be inserted to keep the list sorted?"* That
index is called the **insertion point**.

```python
from bisect import bisect_left, bisect_right
bisect_left([10, 20, 30], 25)   # -> 2   (25 would slot between 20 and 30)
bisect_left([1, 2, 3], 99)      # -> 3   (past the end — never an error)
```

The two variants differ only when `x` is already present — and duplicates are exactly
where they matter:

```python
bisect_left([1, 3, 3, 3, 5], 3)    # -> 1   insert BEFORE the run of equal values
bisect_right([1, 3, 3, 3, 5], 3)   # -> 4   insert AFTER the run
```

So `bisect_left` returns the index of the *first element >= x*, and `bisect_right` the
index of the *first element > x*. Subtract them and you've counted the occurrences of
`x` — or, with different arguments, counted an entire inclusive range with no tree in
sight (Java: `TreeMap.subMap(lo, true, hi, true).size()`):

```python
xs = [1, 2, 3, 4, 5]
bisect_right(xs, 4) - bisect_left(xs, 2)   # -> 3   (counts 2, 3, 4)
```

**Gotcha 1:** `bisect` never checks that the list is sorted — on unsorted input it
cheerfully returns garbage. **Gotcha 2:** it also doesn't tell you whether `x` was
found; it returns a position. Membership test: `i = bisect_left(xs, x)`, then check
`i < len(xs) and xs[i] == x`. **Gotcha 3:** the companion `bisect.insort(xs, x)`
*mutates* `xs` by inserting at the right spot — but the insertion shifts elements, so
it's O(n) per insert, not O(log n). Fine for occasional inserts; for heavy insert
workloads use a heap instead.

### Key functions: one lambda replaces the whole Comparator

Java customizes ordering by transforming the *comparison* (a `Comparator` object with a
`compare(a, b)` method). Python customizes ordering by transforming each *element* into
a sortable stand-in, via the `key=` parameter that `min`, `max`, `sorted`, `nlargest`,
and `nsmallest` all accept. A **lambda** is an anonymous single-expression function:
`lambda p: p[0]` is Java's `p -> p[0]`.

```java
// Java                                          // Python
Collections.min(points,
    Comparator.comparingDouble(                  min(points,
        p -> dist(p, target)));                      key=lambda p: dist(p, target))
```

```python
min(["bb", "a", "ccc"], key=len)               # -> 'a'   (any function works as a key)
min([(1, "x"), (1, "y")], key=lambda p: p[0])  # -> (1, 'x')   equal keys: first one wins
```

Ties go to the **first** element encountered with the minimal key — determinism for free.

For multi-level ordering (Java's `.thenComparing(...)`), return a *tuple* from the key.
Tuples compare **lexicographically** — dictionary order: compare first elements, and
only on a tie move to the second, and so on:

```python
(1, 9) < (2, 0)                        # -> True   (first element decides)
sorted([(2, "b"), (1, "c"), (1, "a")])  # -> [(1, 'a'), (1, 'c'), (2, 'b')]
```

**Gotcha:** `key` must be the *function itself*, not a call — `key=len`, never
`key=len(xs)`. You are handing over a recipe to apply per element, which works because
functions are ordinary values in Python (mental-model shift 1.1 in
[`../LEARNING_POINTS.md`](../LEARNING_POINTS.md)).

### `any` / `all`: short-circuit predicates

`all(iterable)` is True if and only if every element is true; `any(iterable)` if at
least one is. Both **short-circuit** — stop consuming the moment the answer is decided —
exactly like your Java loop with an early `return false`. Feed them a **generator
expression** (the square-bracket list-building syntax from lesson 02, but with
parentheses: it produces a lazy iterator instead of building a list) and no
intermediate list ever exists:

```python
all(a < b for a, b in zip(nums, nums[1:]))   # strictly increasing?
any(x > 100 for x in readings)               # any outlier?
```

The `zip(nums, nums[1:])` trick pairs each element with its successor —
`zip([1, 2, 5, 9], [2, 5, 9])` yields `(1, 2), (2, 5), (5, 9)` — the standard idiom
for "compare adjacent elements."

**Gotcha:** `all([])` is `True` (a claim about zero elements is never violated — the
convention mathematicians call *vacuous truth*), and `any([])` is `False`. Here that's
exactly what the drill wants: an empty list counts as increasing.

## Muscle memory

Type these without thinking. Say the O() out loud in interviews.

```python
for i, x in enumerate(xs, start=1):        # (index, element), no manual counter
dict(zip(keys, values))                    # two parallel lists -> dict
heapq.nlargest(k, xs)                      # top-k, sorted desc, O(n log k)
heapq.heappush(h, x); heapq.heappop(h)     # the queue ops, O(log n) each
bisect_left(xs, x)                         # first index with xs[i] >= x
bisect_right(xs, hi) - bisect_left(xs, lo) # count in inclusive range, O(log n)
min(xs, key=lambda x: ...)                 # Comparator, deleted
all(a < b for a, b in zip(xs, xs[1:]))     # adjacent-pairs check, short-circuits
list(islice(iterable, n))                  # first n of ANYTHING, infinite-safe
yield                                      # inside `while True:` = infinite stream
```

## The drills

### `numbered_lines`

Prefix each line with its 1-based (or `start`-based) number, as `"N: line"`.

```python
[f"{n}: {line}" for n, line in enumerate(["a", "b"], start=1)]
# -> ['1: a', '2: b']
```

**Where you'll see it:** every problem where position matters — interviewers watch for
`range(len(...))` as a Java tell. Real work: numbering frames in an episode dump,
tagging log lines, building `(index, score)` pairs in evaluation scripts.

### `pair_scores`

Zip parallel `names`/`scores` lists into a dict; extra elements on the longer side drop.

```python
dict(zip(["ann", "bob", "cal"], [3, 7]))
# -> {'ann': 3, 'bob': 7}     ("cal" silently dropped — zip stops at the shorter)
```

**Where you'll see it:** building lookup tables from parallel arrays; the same `zip`
transposes a matrix as `zip(*matrix)` (Transpose Matrix, Valid Sudoku column checks).
Real work: pairing joint names with joint positions from a robot state message, zipping
predictions with labels to score accuracy.

### `k_largest`

Return the k largest values, largest first; if `k >= len(nums)` return all, descending.

```python
heapq.nlargest(3, [5, 1, 9, 3, 7])   # -> [9, 7, 5]
heapq.nlargest(10, [2, 8])           # -> [8, 2]   (k too big: fine)
```

**Where you'll see it:** the top-k archetype — Kth Largest Element in an Array
(LeetCode 215), Top K Frequent Elements (347), K Closest Points to Origin (973).
Mention the trade-off out loud: `nlargest` is O(n log k); full sort is O(n log n) and
only fine when k ≈ n. Real work: keeping the k highest-confidence detections from a
perception model, keeping only the k best candidate sequences during model inference
(beam search).

### `merge_sorted_lists`

Merge multiple already-sorted lists into one sorted list.

```python
list(heapq.merge(*[[1, 4, 7], [2, 5], [3, 6, 8]]))
# -> [1, 2, 3, 4, 5, 6, 7, 8]
```

The `*` is **argument unpacking** (the "splat"): `merge` wants each list as a separate
argument — `merge([1,4,7], [2,5], [3,6,8])` — and `*lists` explodes the list-of-lists
into exactly that. It also handles the edge cases: `merge()` with zero lists yields
nothing, and empty sublists are skipped.

**Where you'll see it:** Merge k Sorted Lists (LeetCode 23) — a top-5 interview classic;
say "in production I'd use `heapq.merge`, in the interview I'll build the heap of
`(value, list_index)` if you want internals." Real work: merging time-ordered logs from
several processes, combining per-sensor event streams into one timeline for replay.

### `insertion_index`

Return the leftmost index where `x` can be inserted to keep the list sorted.

```python
bisect_left([10, 20, 30], 25)     # -> 2
bisect_left([1, 3, 3, 3, 5], 3)   # -> 1   (leftmost slot in the run of 3s)
```

**Where you'll see it:** Search Insert Position (LeetCode 35) is literally this
function; bisect_left is also the engine of the O(n log n) Longest Increasing
Subsequence (300) and of Find First and Last Position of Element in Sorted Array (34).
Real work: timestamp alignment — given a sorted list of camera-frame stamps, find the
frame nearest a joint-state stamp: `i = bisect_left(stamps, t)`, then compare
`stamps[i-1]` and `stamps[i]`.

### `count_in_range`

Count values with `lo <= v <= hi` in a sorted list, in O(log n).

```python
xs = [1, 2, 3, 4, 5]
bisect_right(xs, 4) - bisect_left(xs, 2)   # -> 3   (2, 3, 4)
xs = [1, 2, 2, 2, 3]
bisect_right(xs, 2) - bisect_left(xs, 2)   # -> 3   (counts the duplicates)
```

Why the asymmetry: `bisect_left(lo)` finds the first element **>= lo** and
`bisect_right(hi)` the first element **> hi**, so the difference is exactly the
inclusive window — the same variant on both ends miscounts when `lo` or `hi` is present.

**Where you'll see it:** the range-counting archetype behind Find First and Last
Position (34) and time-window queries (Time Based Key-Value Store). Real work: "how
many detections between t=1.0s and t=1.5s?", counting samples in a latency-histogram
bucket — sorted array + two bisects beats a tree structure Python doesn't ship anyway.

### `min_by_distance`

Return the point closest to `target` by straight-line (Euclidean) distance; ties to the
first.

```python
pts = [(0.0, 0.0), (3.0, 4.0), (1.0, 1.0)]
min(pts, key=lambda p: (p[0] - 1.0) ** 2 + (p[1] - 2.0) ** 2)
# -> (1.0, 1.0)     target was (1.0, 2.0)
```

Skip the square root: `sqrt` never changes which of two non-negative numbers is larger,
so comparing *squared* distances picks the same winner and saves a function call per
element. Say that in the interview — it signals numeric fluency.

**Where you'll see it:** the single-nearest version of K Closest Points to Origin (973);
any "find the best candidate under a scoring function" question. Real work: nearest
waypoint on a path, closest obstacle to the gripper, greedy detection-to-track matching
— `min(candidates, key=...)` is one line each time.

### `all_increasing`

Return True exactly when the list is strictly increasing; empty and single-element
lists count as increasing.

```python
all(a < b for a, b in zip([1, 2, 5, 9], [1, 2, 5, 9][1:]))   # -> True
all(a < b for a, b in zip([1, 2, 2, 3], [1, 2, 2, 3][1:]))   # -> False (2 < 2 fails)
```

Empty and single-element inputs produce zero pairs, and `all()` of nothing is `True` —
the edge case handles itself.

**Where you'll see it:** Monotonic Array and the validation step inside Valid Mountain
Array; the adjacent-pairs `zip(xs, xs[1:])` pattern also drives "max adjacent gap" and
diff-style questions. Real work: sanity-checking that log timestamps never go backward
before replay, asserting a planned joint trajectory has strictly advancing time steps.

### `take`

Return the first `n` items of ANY iterable as a list; fewer available means return all.

```python
from itertools import islice, count
list(islice([1, 2, 3, 4], 2))    # -> [1, 2]
list(islice(count(100), 3))      # -> [100, 101, 102]   (count(100) counts up forever)
list(islice([1, 2], 5))          # -> [1, 2]            (short input: no error)
```

**Where you'll see it:** rarely a whole problem, but the difference between a correct
and a hanging solution whenever a generator is involved — a favorite follow-up is "your
function returns a stream; show me the first 5." Real work: smoke-testing a pipeline by
taking 10 batches from a loader, sampling the head of a huge log without reading it all.

### `fibonacci`

An INFINITE generator yielding 0, 1, 1, 2, 3, 5, ... — it never returns; callers slice.

```python
def fibonacci():
    a, b = 0, 1
    while True:
        yield a
        a, b = b, a + b

list(islice(fibonacci(), 10))   # -> [0, 1, 1, 2, 3, 5, 8, 13, 21, 34]
```

Note `a, b = b, a + b`: both right-hand sides are evaluated *before* either assignment
happens — the same tuple unpacking as the `a, b = b, a` swap, so no temp variable.

**Where you'll see it:** the design-an-iterator family — Peeking Iterator, Flatten
Nested List Iterator — where a generator replaces the state-machine class Java forces
on you; also any "process a stream you can't fit in memory" discussion. Real work:
PyTorch-style data loaders are exactly this shape (yield batches forever, training loop
slices), as are frame-by-frame readers over multi-gigabyte robot logs.

## How to practice

Write your attempts in `starter.py`, then run the tests against them:

```bash
PRACTICE=1 uv run pytest ramp_up/python/07_iteration_heapq_bisect -v
```

Drop the `PRACTICE=1` to run the tests against `solution.py` instead.
