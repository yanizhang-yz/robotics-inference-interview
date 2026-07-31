# 01 — Collections and Slicing

This lesson covers the two tools behind most everyday sequence work in Python: the `list`
type and the slice syntax `lst[start:stop:step]`. After it, you should be able to rotate,
reverse, chunk, sample, and multi-key-sort a list in one line each, without mutating the
input, and know exactly which interview problems those one-liners solve. Every term is
defined at first use, and all outputs shown in comments were run and verified.

## The problem this lesson solves

Most list manipulation — take the last n items, reverse without destroying the original,
rotate by k, split into batches, sort by two criteria at once — *can* be written as index
loops with careful bounds arithmetic. That version is slow to type, easy to get wrong at
the edges, and instantly reads as unidiomatic in an interview. Python compresses each of
those operations into a one-line idiom built on slicing, unpacking, and key functions —
and the idiomatic versions return new lists instead of mutating, which is exactly the
behavior the tests in this folder check for.

## The lesson

### One `list` for everything

Many languages make you juggle two families of sequences: a fixed-size primitive array
for compact storage and a resizable object container layered on top, with wrapping and
conversion rules where the two meet. Python has none of this. There is exactly one
everyday sequence type, `list`. It is resizable, and it is **heterogeneous** — meaning
its elements may be of different types in the same list:

```python
nums = [5, 1, 4, 2, 3]      # a list literal — no `new`, no type declarations
mixed = [1, "two", 3.0]     # legal: int, str, float in one list
```

Python has two comparison operators, and they answer different questions: `==` compares
*values* (do these objects hold the same contents?), and the keyword `is` compares
*identity* (are these literally the same object in memory?). You will almost always
want `==`.

```python
[1, 2] == [1, 2]   # -> True    same contents
[1, 2] is [1, 2]   # -> False   two separate objects
x = [1, 2]; y = x
x is y             # -> True    y is an alias — the SAME object
```

One rule sits underneath all of this: **variables are references**. A Python name is a
pointer to an object, never a copy of it, so assigning a list to a second name creates
an **alias** — two names for the same object:

```python
u = [1, 2]
v = u             # alias, NOT a copy
v.append(3)
u                 # -> [1, 2, 3]  (u saw the change — same list)
```

If you want an actual copy, you must ask for one: `list(u)` or `u[:]` (both explained
below). This matters for every drill, because the tests verify you did not mutate the input.

### Indexing: negative numbers count from the end

`lst[i]` fetches the element at index `i`, counting from 0. The new part: a **negative
index** counts backwards from the end, with `-1` meaning the last element. This kills the
`lst[len(lst) - 1]` dance forever:

```python
lst = [10, 20, 30]
lst[-1]     # -> 30   (last)
lst[-2]     # -> 20   (second to last)
lst[10]     # raises IndexError — out-of-range single indexes still throw
```

Note the last line: a *single* out-of-range index throws an exception. Only slices
(next section) are forgiving.

### Slicing: `lst[start:stop:step]`

A **slice** takes a sub-sequence in one expression: `lst[start:stop:step]`. The rules:

- **Half-open interval**: includes `start`, excludes `stop` — the same convention as
  `range`. `lst[1:3]` is elements 1 and 2.
- Every part is optional. `start` defaults to 0, `stop` to the end, `step` to 1.
- **`step`** (also called the **stride**) is how far to jump between elements. Step 2 takes
  every other element; step -1 walks backwards.
- Slices **clamp** instead of throwing: indices past the end are silently pulled back to
  the boundary. There is no exception a slice can raise for being out of range.
- A slice always builds and returns a **new list** — an independent copy of that range.
  (In some languages a sub-list is a live *view*: mutate the view, mutate the original.
  A Python slice never does that.)

```python
lst = [1, 2, 3, 4, 5]
lst[1:3]      # -> [2, 3]        half-open: index 1 and 2
lst[:3]       # -> [1, 2, 3]     first three
lst[2:]       # -> [3, 4, 5]     from index 2 to the end
lst[-2:]      # -> [4, 5]        last two
lst[::2]      # -> [1, 3, 5]     every other (step 2)
lst[::-1]     # -> [5, 4, 3, 2, 1]  reversed copy (step -1)
lst[5:100]    # -> []            clamps — no exception, ever
lst[:]        # -> [1, 2, 3, 4, 5]  full copy (all defaults)
```

One trap: `-0` is just `0`, so `lst[-0:]` is `lst[0:]` — the *whole* list, not "last zero
elements". Any "last n" code must special-case `n == 0`.

### Copies are shallow

`lst[:]` and `list(lst)` both make a **shallow copy**: a new outer list whose slots point
at the *same* element objects as the original. For flat lists of numbers or strings that is
all you ever need. For nested lists, the inner lists are shared:

```python
outer = [[1, 2], [3]]
copy = outer[:]           # new outer list, SAME inner lists
copy[0].append(99)
outer                     # -> [[1, 2, 99], [3]]  — the original changed!
copy[0] = [7]             # but REPLACING a slot only touches the copy — outer unaffected
```

A **deep copy** (recursively copying the inner objects too) exists as
`copy.deepcopy(x)` but is rarely needed in interviews. See Gotcha 9 in
[`../LEARNING_POINTS.md`](../LEARNING_POINTS.md).

### Mutate in place vs. return a copy — the `None` trap

"**In place**" means modifying the existing object instead of building a new one. Python
gives you both verbs for every operation, with a naming convention:

| copying (returns a new list)   | mutating in place (returns nothing) |
|--------------------------------|--------------------------------------|
| `sorted(lst)`                  | `lst.sort()`                         |
| `lst[::-1]`                    | `lst.reverse()`                      |

"Returns nothing" in Python means returning **`None`** — Python's "no value", and the
implicit return value of any function without a `return` statement. This is a classic
newcomer bug: **if you write `result = lst.sort()`, you stored `None`, because
`.sort()` mutates and returns nothing** — and the crash happens later, wherever `result`
is first used.

```python
lst = [3, 1, 2]
result = lst.sort()   # WRONG: result is None; lst is now [1, 2, 3]
result = sorted(lst)  # RIGHT: result is a new sorted list; lst untouched
```

Default to the copying forms. Reach for `.sort()` only when you own the list and want to
avoid a copy.

### Tuples and unpacking

A **tuple** is a list that is **immutable** — frozen after creation, no appends, no item
assignment. Written with parentheses (or nothing at all: `1, 2` is already a tuple).
It is Python's built-in fixed-size record — no class definition needed:

```python
t = (1, 2)
t[0] = 5          # raises TypeError — tuples cannot be modified
```

**Unpacking** assigns a sequence to multiple names at once: `a, b = pair`. Its killer app
is the swap. The whole right-hand side is evaluated first (it builds a tuple), *then*
assigned to the left-hand names — which is why no temp variable is needed:

```python
a, b = 1, 2
a, b = b, a            # -> a == 2, b == 1   (no temp variable)
first, *rest = [1, 2, 3, 4]   # -> first == 1, rest == [2, 3, 4]
```

The `*` in unpacking means "collect the remainder into a list" — a varargs-style catch-all.

### Sorting: key functions

Some languages sort by transforming *comparisons*: you supply a comparator function that
takes two elements and says which is smaller. Python sorts by transforming *elements*:
you pass a **key function** — called once per element — and Python orders the elements
by their keys. `lambda` is Python's inline anonymous-function syntax: `lambda w: expr`
defines an unnamed function that takes `w` and returns `expr`.

Multi-level sorting falls out for free because of one fact: **tuples compare
lexicographically**. "Lexicographic" is dictionary order — compare first components; only
on a tie compare the second; and so on. So a tuple-valued key IS a full
sort-by-this-then-by-that chain:

```python
words = ["banana", "kiwi", "fig", "date", "apple"]
sorted(words, key=lambda w: (len(w), w))
# -> ['fig', 'date', 'kiwi', 'apple', 'banana']   (by length, ties alphabetical)
```

Two more tools you will use constantly:

- **Descending**: pass `reverse=True`, or negate a numeric key component. Negation lets you
  mix directions in one key — count descending, then name ascending:

  ```python
  pairs = [("a", 2), ("b", 5), ("c", 2)]
  sorted(pairs, key=lambda p: (-p[1], p[0]))
  # -> [('b', 5), ('a', 2), ('c', 2)]   (5 first; the tied 2s fall back to name order)
  ```
- **Stability**: Python's sort is **stable** — elements with equal keys keep their original
  relative order. `sorted(["bb", "aa", "cc"], key=len)` returns `['bb', 'aa', 'cc']`
  unchanged, because all keys tie. Stability is what makes multi-pass sorting (sort by a
  secondary key first, then the primary) work at all.

### `zip`: lockstep iteration

`zip(a, b)` walks two (or more) sequences in parallel, yielding pairs, and **stops at the
shorter input** — no manual length bookkeeping. It is **lazy**: it produces pairs on
demand rather than building a list up front, so wrap it in `list(...)` if you want to
look at the result directly:

```python
list(zip([1, 2, 3], ["a"]))       # -> [(1, 'a')]  — stopped after the shorter list
list(zip([1, 3, 5], [2, 4, 6]))   # -> [(1, 2), (3, 4), (5, 6)]
```

### List comprehensions — the minimum dose

A **comprehension** builds a list from a loop in one expression — filter, transform, and
collect in a single line. Two `for` clauses read left to right, exactly like the
equivalent nested loops:

```python
[x * 2 for x in [1, 2, 3]]                    # -> [2, 4, 6]
[x for sub in [[1, 2], [3]] for x in sub]     # -> [1, 2, 3]  (outer loop first, then inner)
```

That second shape (flattening) is all this lesson needs; comprehensions get their own full
treatment in drill set `02_comprehensions_and_generators`.

One last idiom you will see in the solutions: **truthiness**. Empty collections count as
`False` in a boolean context, so `if not lst:` is the idiomatic emptiness check:

```python
not []        # -> True    an empty list is "falsy"
not [1, 2]    # -> False   a non-empty list is "truthy"
```

(Gotcha 3 in [`../LEARNING_POINTS.md`](../LEARNING_POINTS.md).)

## Muscle memory

Type these until they come out without thinking:

```python
lst[-1]                                   # last element
lst[-n:]                                  # last n elements (n > 0!)
lst[::2]                                  # every other element
lst[::-1]                                 # reversed copy
lst[k:] + lst[:k]                         # rotate left by k (k already reduced % len)
lst[i:i + size]                           # one chunk; clamps at the end
a, b = b, a                               # swap, no temp
sorted(xs, key=lambda x: (x[0], -x[1]))   # multi-key sort via tuple key
[x for sub in nested for x in sub]        # flatten one level
list(lst)                                 # shallow copy (== lst[:])
```

## The drills

Work through `starter.py` top to bottom. Each drill below: the task, the idiom, and where
that exact idiom shows up in interviews and in robotics/ML code.

### `rotate_left(lst, k)`

Return a new list rotated left by `k`; `k` may exceed the length; empty stays empty.

```python
lst, k = [1, 2, 3, 4, 5], 7
k %= len(lst)              # 7 % 5 == 2 — a k bigger than the list wraps around
lst[k:] + lst[:k]          # -> [3, 4, 5, 1, 2]
```

Guard the empty list first (`if not lst: return []`) because `k % 0` raises
`ZeroDivisionError`. Bonus: Python's `%` never returns a negative result for a positive
divisor (`-1 % 5 == 4`; in many languages the sign of `%` follows the left operand and
`-1 % 5` is `-1`), so negative rotations wrap correctly for free.

**Where you'll see it:** this is literally "Rotate Array" (LeetCode 189, usually asked as
rotate-right — same idiom, flipped slice). In robotics it is the ring/circular buffer
pattern: shifting a fixed-size window of recent sensor or IMU (inertial measurement unit)
readings.

### `last_n(lst, n)`

Return the last `n` items as a new list; `n == 0` returns `[]`.

```python
[1, 2, 3, 4, 5][-2:]    # -> [4, 5]
[1, 2][-10:]            # -> [1, 2]   (clamps — more than we have is fine)
```

The trap is `n == 0`: `lst[-0:]` is the whole list, so return `[]` explicitly for that case.

**Where you'll see it:** the tail end of every sliding-window problem (a sliding window is a
fixed-size range that slides forward over a stream; "Moving Average from Data Stream" is the
canonical one) and anywhere you keep "the most recent N": last N camera
frames for a temporal model, last N loss values for a smoothed training curve, log tails.

### `every_other(lst)`

Return the items at even indices (0, 2, 4, ...).

```python
[10, 20, 30, 40, 50][::2]    # -> [10, 30, 50]
[10, 20, 30, 40, 50][1::2]   # -> [20, 40]      (odd indices — know both)
```

**Where you'll see it:** downsampling — processing every Nth camera frame because inference
is slower than the camera, decimating a high-rate signal before plotting. The `step` concept
returns with real force in `08_numpy_essentials`, where strided slicing works on arrays
without copying.

### `reverse_copy(lst)`

Return a reversed copy; the input must not be modified.

```python
[1, 2, 3][::-1]        # -> [3, 2, 1], input untouched
s = "racecar"
s == s[::-1]           # -> True — the one-line palindrome check
```

Remember the twin: `lst.reverse()` mutates and returns `None` — the `result = lst.sort()`
trap in reverse clothing.

**Where you'll see it:** "Reverse String", "Valid Palindrome" (LeetCode 125) and every
palindrome variant, "Reverse Words in a String". In robotics: reversing a planned waypoint
path to drive the return leg, flipping a trajectory before replay.

### `swap_ends(lst)`

Return a copy with first and last elements swapped; lists shorter than 2 come back as a copy.

```python
out = list(lst)                        # copy FIRST — the test checks lst is untouched
out[0], out[-1] = out[-1], out[0]      # [1, 2, 3, 4] -> [4, 2, 3, 1]
```

**Where you'll see it:** the swap is the atom of every two-pointer/in-place problem
(two-pointer = two indices walking through the list, often from both ends toward the
middle) — reversing a list in place, the partition step of quicksort, "Sort Colors" (LeetCode 75,
the Dutch National Flag partition). Interviewers watch whether you reach for a temp
variable; the tuple swap marks you as fluent.

### `interleave(a, b)`

Return `[a[0], b[0], a[1], b[1], ...]`, stopping at the shorter input.

```python
[x for pair in zip([1, 3, 5], [2, 4, 6]) for x in pair]
# -> [1, 2, 3, 4, 5, 6]
# zip([1, 2, 3], ["a"]) yields just (1, 'a') — the length check is built in
```

**Where you'll see it:** the "Shuffle the Array" archetype, and `zip` itself is one of the
most-typed functions in ML code: pairing timestamps with sensor readings, joint names with
joint angles, predictions with labels. Party trick worth memorizing: `list(zip(*matrix))`
transposes a matrix — `[[1, 2, 3], [4, 5, 6]]` becomes `[(1, 4), (2, 5), (3, 6)]`. (Here `*`
means the opposite of the unpacking `*` above: in a *call*, `zip(*rows)` spreads the list's
items out as separate arguments, i.e. `zip(rows[0], rows[1])`.)

### `sort_by_length_then_alpha(words)`

Return words sorted by length, ties alphabetical, without mutating the input.

```python
sorted(["banana", "kiwi", "fig", "date", "apple"], key=lambda w: (len(w), w))
# -> ['fig', 'date', 'kiwi', 'apple', 'banana']
```

The tuple key is the entire sort-by-length-then-alphabetical chain in one expression.

**Where you'll see it:** custom-sort is its own interview genre — "Merge Intervals"
(sort by start first), "K Closest Points to Origin" (sort by distance), "Top K Frequent
Elements" (sort by `(-count, word)`), "Sort Characters By Frequency". In robotics/ML:
ordering detections by confidence then class, ranking grasp candidates by score then
reachability.

### `top_k_smallest(nums, k)`

Return the `k` smallest numbers ascending; `k` past the end returns everything sorted.

```python
sorted([5, 1, 4, 2, 3])[:3]    # -> [1, 2, 3]
sorted([3, 1])[:10]            # -> [1, 3]   (slice clamps — no min(k, len) needed)
```

`sorted()` copies, the slice clamps: zero bounds bookkeeping. This is the O(n log n)
answer; the O(n log k) heap version (`heapq.nsmallest`) is drill set
`07_iteration_heapq_bisect` — in an interview, give this one first and *mention* the heap.

**Where you'll see it:** "Kth Largest Element in an Array" (LeetCode 215), "Top K Frequent
Elements" (LeetCode 347), "K Closest Points to Origin" (LeetCode 973). Real work: keeping
the k highest-confidence object detections, picking the top-k tokens from a model's
output scores.

### `chunk(lst, size)`

Split into consecutive chunks of `size`; last chunk may be shorter; empty in, empty out.

```python
[[1, 2, 3, 4, 5][i:i + 2] for i in range(0, 5, 2)]
# -> [[1, 2], [3, 4], [5]]      range(0, 5, 2) is 0, 2, 4; the last slice clamps
```

`range(start, stop, step)` follows the same half-open, stepped rules as slicing — one
mental model for both.

**Where you'll see it:** mini-batching, everywhere — cutting a dataset or a stream of
frames into batches for model inference is this exact comprehension (it *is*
what a data loader does at its core). Also pagination, and reshaping a flat list into
rows of a matrix.

### `flatten_one_level(nested)`

Flatten a list of lists into one list — one level deep only.

```python
[x for sub in [[1, 2], [3], [4, 5]] for x in sub]   # -> [1, 2, 3, 4, 5]
[x for sub in [[1, [2]], [3]] for x in sub]         # -> [1, [2], 3]  (inner nesting kept)
```

Read the `for` clauses left to right: outer loop, then inner loop — the same order you
would write the nested `for` statements.

**Where you'll see it:** the deep-recursive version is "Flatten Nested List Iterator";
the one-level version is a daily chore — merging per-camera detection lists into one,
gathering results from parallel workers, combining per-file token lists. Interviewers use
it as a comprehension fluency check: writing the two-`for` version without hesitation is
the point.

## How to practice

Write your attempts in `starter.py`, then run the tests against them:

```bash
PRACTICE=1 uv run pytest ramp_up/python/01_collections_and_slicing -v
```

Drop the `PRACTICE=1` to run the tests against `solution.py` instead.
