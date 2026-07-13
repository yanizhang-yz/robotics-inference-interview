# 02 — Comprehensions and Generators

This lesson replaces `java.util.stream` in your head. After it, you will be able to
collapse any `stream().filter().map().collect()` pipeline into one line of Python,
build dicts and sets the same way, write a lazy iterator in three lines instead of a
`hasNext()/next()` class, and read the `zip(*matrix)` idioms that Python interviewers
treat as fluency signals. Every term is defined on first use, and every snippet's
output was executed and checked — trust the `# ->` comments.

## The Java you know

This is how you process a collection today. Keep it in view; the whole lesson maps
onto these five moves.

```java
List<Integer> nums = List.of(1, 2, 3, 4, 5);

// filter + map + collect into a List
List<Integer> squares = nums.stream()
    .filter(n -> n % 2 == 0)
    .map(n -> n * n)
    .collect(Collectors.toList());          // [4, 16]

// collect into a Map
Map<String, Integer> lengths = words.stream()
    .collect(Collectors.toMap(w -> w, String::length));

// flatten nested lists
List<Integer> flat = matrix.stream()
    .flatMap(List::stream)
    .collect(Collectors.toList());

// first match or fallback
int firstEven = nums.stream()
    .filter(n -> n % 2 == 0)
    .findFirst().orElse(-1);                // 2
```

And when you need laziness outside a stream, you hand-write an `Iterator<T>` class
with state fields. Hold that thought for the generator section.

## The lesson

### 1. Comprehensions: the whole pipeline in one expression

A **comprehension** is Python's syntax for "build a collection by transforming and
filtering another one." The name comes from mathematical **set-builder notation** —
the `{ n² | n ∈ nums, n even }` style you saw in school. It reads as a description
of the result, not as a chain of method calls:

```java
// Java: a pipeline read top to bottom
List<Integer> out = nums.stream()
    .filter(n -> n % 2 == 0)
    .map(n -> n * n)
    .collect(Collectors.toList());
```

```python
# Python: one expression
out = [n * n for n in [1, 2, 3, 4, 5] if n % 2 == 0]   # -> [4, 16]
```

Anatomy, mapped onto the stream calls you know:

```python
[ n * n     for n in nums     if n % 2 == 0 ]
# ^expr      ^loop             ^filter
# .map()     the source        .filter()
# the [ ] brackets are the .collect(Collectors.toList())
```

Two execution-model differences from streams:

- Java streams are **lazy** (nothing runs until a *terminal operation* like
  `collect()` fires the pipeline). A comprehension is **eager**: it runs
  immediately, top to bottom, and hands you the finished list. The lazy variant
  exists too — that's generators, section 3.
- The loop variable is scoped to the comprehension — no leaked loop counters. Once
  the comprehension has run, the name `n` does not exist outside it:

  ```python
  nums = [1, 2, 3]
  [n * n for n in nums]
  n   # NameError: name 'n' is not defined
  ```

**Gotcha — where the `if` goes changes what it means.** A trailing `if` *filters*
(drops elements). An `if/else` used to *transform* each element must go at the
front, before the `for`, because there it is a conditional expression (Python's
ternary operator, its version of Java's `cond ? a : b`):

```python
[x for x in [-5, 3, -1, 8] if x > 0]           # filter -> [3, 8]
[x if x > 0 else 0 for x in [-5, 3, -1, 8]]    # transform -> [0, 3, 0, 8]
[x for x in xs if x > 0 else 0]                # SyntaxError — no trailing else
```

### 2. The bracket picks the container — there are no Collectors

In Java you pick the output container by choosing a collector: `toList()`,
`toSet()`, `toMap(...)`. In Python the *bracket around the comprehension* decides:

```python
[w.upper() for w in words]          # list  — like Collectors.toList()
{w.upper() for w in words}          # set   — like Collectors.toSet()
{w: len(w) for w in words}          # dict  — like Collectors.toMap()
(w.upper() for w in words)          # generator — lazy, see section 3
```

A **dict** is Python's `HashMap`: key-value pairs with O(1) average lookup. A
**set** is its `HashSet`: unordered unique values. Both require keys/elements to be
**hashable** — meaning the object can be converted to a stable integer (a *hash*)
used to find its storage bucket. Numbers, strings, and tuples are hashable; lists
are not (they can mutate, which would silently change their bucket).

Three behavioral differences that pay off in interviews:

- **Duplicate keys: Python keeps the last, Java throws.**
  `Collectors.toMap` throws `IllegalStateException` on a duplicate key (documented
  behavior). A dict comprehension silently overwrites:

  ```python
  {k: v for k, v in [("a", 1), ("b", 2), ("a", 3)]}   # -> {'a': 3, 'b': 2}
  ```

- **Python dicts remember insertion order.** Iterating a `HashMap` gives keys in
  effectively arbitrary order; you reach for `LinkedHashMap` to preserve insertion
  order. Every Python `dict` preserves insertion order, guaranteed by the language
  since 3.7:

  ```python
  list({w: len(w) for w in ["zebra", "ant", "mole"]})   # -> ['zebra', 'ant', 'mole']
  ```

- **`{}` is an empty dict, NOT an empty set** — the braces were taken by dicts
  first. Write `set()` for an empty set. (`type({})` is `dict`; verified.)

One Python-ism you need for the drills: **truthiness**. Every value can stand in
for a boolean. Empty things — `""`, `[]`, `{}`, `0`, `None` — count as `False`
("**falsy**"); non-empty things count as `True`. So `if w` inside a comprehension
is the idiomatic way to skip empty strings — no `!w.isEmpty()` call:

```python
{w[0].lower() for w in ["Apple", "avocado", "", "Banana"] if w}   # -> {'a', 'b'}
```

### 3. Generators: a lazy iterator without the boilerplate class

Vocabulary first, because Python's docs use these constantly:

- An **iterable** is anything a `for` loop can walk: lists, strings, dicts, files.
  (Java analog: `Iterable<T>`.)
- An **iterator** is the cursor doing the walking — it hands out one element per
  request and remembers its position. (Java analog: `Iterator<T>`.)
- A **generator** is an iterator you get for free by writing a function that uses
  the `yield` keyword. `yield` means "emit this value, then FREEZE right here until
  someone asks for the next one." All local variables survive the freeze.

Side by side — counting down:

```java
// Java: a class with explicit cursor state
class Countdown implements Iterator<Integer> {
    private int current;
    Countdown(int n) { this.current = n; }
    public boolean hasNext() { return current >= 1; }
    public Integer next()    { return current--; }
}
```

```python
# Python: the local variable i IS the cursor state
def countdown(n):
    for i in range(n, 0, -1):   # range(4, 0, -1) counts 4, 3, 2, 1
        yield i

list(countdown(4))   # -> [4, 3, 2, 1]
```

Three facts about generators that will bite you if you learn them the hard way:

- **Calling a generator function runs NO body code.** The presence of `yield`
  anywhere in the body changes what calling the function means: it just builds the
  iterator object. The body starts executing on the first `next()` and pauses at
  each `yield`:

  ```python
  def gen():
      print("body running")   # first line of the body
      yield 1

  g = gen()   # prints NOTHING — calling it ran no body code, just built the iterator
  next(g)     # NOW prints "body running", then returns 1
  ```

  So `countdown(1_000_000_000)` returns instantly and costs no memory — values are
  produced one at a time, on demand.
- **Generators are single-use, and exhaustion is SILENT.** A consumed Java stream
  at least throws `IllegalStateException` ("stream has already been operated upon")
  if you touch it again. An exhausted generator just quietly yields nothing:

  ```python
  g = (n * n for n in [1, 2, 3])
  list(g)   # -> [1, 4, 9]
  list(g)   # -> []        <- no error, no warning, just empty
  ```

  If you need multiple passes, **materialize** it once — force the lazy iterator
  to produce everything and store it in a real list: `results = list(g)`.
- **No `len()`, no indexing.** A generator does not know its length — it hasn't
  produced its values yet — so asking gives you an error, not a number:

  ```python
  len(x for x in [1, 2, 3])   # TypeError: object of type 'generator' has no len()
  ```

  `list()` it first if you need a length or an index.

### 4. Generator expressions and `next()` — `findFirst().orElse()` in one call

A **generator expression** ("genexpr") is a comprehension with parentheses instead
of brackets: lazy, produces items on demand, never builds the full list. When it is
the only argument to a function call, the call's own parentheses suffice:

```python
sum(x * x for x in [1, 2, 3])   # -> 14, no intermediate list ever exists
```

`next(iterator)` pulls one value. On an empty/exhausted iterator it raises
**`StopIteration`** — the exception Python uses internally as the "no more
elements" signal (it is what ends every `for` loop; you rarely catch it yourself).
Pass a second argument and `next` returns that default instead of raising:

```python
next(iter([]))        # raises StopIteration
next(iter([]), -1)    # -> -1
```

Combine the two and you have `findFirst().orElse()` with short-circuiting — a
**predicate** (a function returning True/False) stops being evaluated at the first
hit because the genexpr is lazy:

```python
next((x for x in [1, 3, 4, 6] if x % 2 == 0), -1)   # -> 4, never looks at 6
```

You'll usually write the predicate inline as a **lambda** — Python's anonymous
function, its `n -> n * 2`: `lambda n: n * 2`. Same idea, `lambda` keyword, colon
instead of arrow, no type declarations.

### 5. `zip` and `*` unpacking — the idioms interviewers watch for

`zip(a, b)` staples iterables together element-wise into **tuples** — a tuple being
Python's immutable fixed-size sequence, `(1, 4)`, the built-in `Pair`/record that
Java makes you define a class for. Two properties to memorize:

```python
list(zip([1, 2, 3], [10, 20]))   # -> [(1, 10), (2, 20)]  — stops at the SHORTER input
zip([1], [2])                    # a lazy zip object, not a list — wrap in list()
```

`*` before an argument is **argument unpacking**: it spreads a collection into
separate positional arguments, so `f(*[a, b, c])` means `f(a, b, c)`. Point it at a
matrix (a list of row-lists) and `zip` receives every row as its own argument, then
staples them back together column-wise. That is a transpose, in one line, with no
`new int[cols][rows]` allocation dance:

```python
m = [[1, 2, 3],
     [4, 5, 6]]
list(zip(*m))                      # -> [(1, 4), (2, 5), (3, 6)]
[list(col) for col in zip(*m)]     # -> [[1, 4], [2, 5], [3, 6]]  (lists, not tuples)
```

The same trick run backwards is **unzipping** — splitting a list of pairs into two
parallel sequences:

```python
pairs = [(1, "a"), (2, "b"), (3, "c")]
nums, letters = zip(*pairs)        # -> (1, 2, 3) and ('a', 'b', 'c')
```

And zipping a list against its own off-by-one slice walks consecutive pairs —
`lst[1:]` is "everything from index 1 on", and the stops-at-shorter rule makes the
bounds work out with no `i < n - 1` bookkeeping:

```python
lst = [1, 2, 3]
list(zip(lst, lst[1:]))            # -> [(1, 2), (2, 3)]
```

(Python 3.10+ ships this as `itertools.pairwise(lst)` — same output. Knowing the
self-zip spelling is still expected.)

### 6. `itertools`: the stream utility belt

`itertools` is a standard-library module of building blocks for iteration — the
closest thing to the static helpers scattered across `Stream`, `IntStream`, and
`Collectors`. Everything it returns is lazy, so wrap in `list()` to materialize.
The two you need for the drills:

```python
import itertools
import operator   # each Python operator as a plain function: operator.mul is *,
                  # operator.add is + — Python's stand-in for Integer::sum

# accumulate: running totals (prefix sums); pass a function to change the operation
list(itertools.accumulate([1, 2, 3, 4]))                 # -> [1, 3, 6, 10]
list(itertools.accumulate([1, 2, 3, 4], operator.mul))   # -> [1, 2, 6, 24]

# product: Cartesian product — every combination, first argument varying slowest
list(itertools.product(["red", "blue"], ["S", "M"]))
# -> [('red', 'S'), ('red', 'M'), ('blue', 'S'), ('blue', 'M')]
```

A **Cartesian product** is the set of all pairs (or triples, ...) you can form by
taking one element from each input — exactly what nested for-loops produce.
`product(xs, repeat=2)` pairs a collection with itself: `product([-1, 0, 1],
repeat=2)` yields all 9 `(row, col)` offsets around a grid cell — that is the 8
neighbors plus `(0, 0)`, the cell itself, which you filter out — a move you will
use in every grid-search problem.

Deep dives on adjacent topics (lazy built-ins, LEGB scoping, the full idiom sheet)
live in [`../LEARNING_POINTS.md`](../LEARNING_POINTS.md) — this lesson stands alone
for comprehensions and generators.

## Muscle memory

Type these until your fingers know them:

```python
[f(x) for x in xs if cond(x)]              # filter + map + collect
{k(x): v(x) for x in xs}                   # collect into a dict
{f(x) for x in xs}                         # collect into a set
[x for row in matrix for x in row]         # flatten — for-clauses in nested-loop order
[list(col) for col in zip(*matrix)]        # transpose
list(zip(lst, lst[1:]))                    # consecutive pairs
next((x for x in xs if p(x)), default)     # findFirst().orElse(default)
sum(f(x) for x in xs)                      # aggregate without building a list
list(itertools.accumulate(nums))           # prefix sums
list(itertools.product(colors, sizes))     # all combinations
```

## The drills

### `squares_of_evens`

Return the squares of the even numbers, preserving order.

```python
[n * n for n in [1, 2, 3, 4, 5] if n % 2 == 0]   # -> [4, 16]
```

Where you'll see it: this filter-map shape is the warm-up of half of all easy array
problems — the map step of "Squares of a Sorted Array" is exactly the `n * n`
expression here. In robotics/ML code it is every preprocessing filter: keep the
valid sensor readings, square the errors, drop the NaNs — one line per cleaning
step.

### `word_length_map`

Return a dict mapping each word to its length.

```python
{w: len(w) for w in ["hi", "world"]}   # -> {'hi': 2, 'world': 5}
```

Remember: duplicate keys keep the *last* value silently, where `Collectors.toMap`
throws.

Where you'll see it: building a lookup table is the setup move of "Two Sum",
"Group Anagrams", and "Top K Frequent Elements" — interviewers watch whether you
build the dict in one comprehension or fumble with `put`-style loops. In ML work
this is every vocab map (`{word: index}`), label map, and config table.

### `unique_first_letters`

Return the set of lowercased first letters, ignoring empty strings.

```python
{w[0].lower() for w in ["Apple", "avocado", "", "Banana"] if w}   # -> {'a', 'b'}
```

The `if w` uses truthiness: an empty string is falsy, so it filters `""` out and
makes `w[0]` safe.

Where you'll see it: set-based dedup and membership is the core of "Contains
Duplicate" and "Longest Substring Without Repeating Characters", and the `seen`
set in every graph traversal (BFS/DFS — breadth-/depth-first search) you will
ever write. The guard-then-index pattern (`if w`
before `w[0]`) is the standard defensive move on messy real-world data.

### `transpose`

Return the transpose of a rectangular matrix (list of rows becomes list of columns).

```python
[list(col) for col in zip(*[[1, 2, 3], [4, 5, 6]])]   # -> [[1, 4], [2, 5], [3, 6]]
```

Where you'll see it: "Transpose Matrix" verbatim, and "Rotate Image" — the
standard solution is transpose-then-reverse-rows, and saying "zip star" out loud
is a fluency signal. In ML pipelines the same idiom *unzips* batches:
`inputs, labels = zip(*batch)` splits a list of (image, label) pairs into two
parallel sequences — the core of the function that assembles samples into a
training batch (PyTorch calls it `collate_fn`).

### `flatten_matrix`

Flatten a list of rows into one flat list, row-major order (**row-major** = walk
row 0 left to right, then row 1, ...).

```python
[x for row in [[1, 2], [3, 4], [5]] for x in row]   # -> [1, 2, 3, 4, 5]
```

The for-clauses appear in the SAME order as the equivalent nested loops — outer
loop first. Reading them inside-out is the #1 nested-comprehension mistake.

Where you'll see it: the comprehension form of `flatMap`, tested by "Flatten 2D
Vector" and needed mid-solution whenever a grid must become a flat list. In
practice: flattening per-camera detection lists into one list, grid maps into 1-D
arrays.

### `running_totals`

Return the running (prefix) sums: each output element is the sum of all inputs up
to and including that position.

```python
list(itertools.accumulate([1, 2, 3, 4]))   # -> [1, 3, 6, 10]
```

Where you'll see it: **prefix sums** are a top-five interview technique — "Subarray
Sum Equals K", "Range Sum Query — Immutable", and (as prefix *products*) "Product
of Array Except Self". Knowing `accumulate` exists saves you the accumulator loop.
In robotics: integrating velocities into positions, cumulative time offsets along a
trajectory, cumulative distributions for weighted sampling.

### `countdown`

Write a generator yielding `n, n-1, ..., 1`; `countdown(0)` yields nothing.

```python
def countdown(n):
    for i in range(n, 0, -1):
        yield i

list(countdown(4))   # -> [4, 3, 2, 1]
```

The tests also check laziness (pull two values with `next`, the rest stay pending)
and single-use (a second `list()` of the same generator gives `[]`).

Where you'll see it: interviewers probe generators via design questions —
"Flatten Nested List Iterator" and "Peeking Iterator" are Iterator-design problems
that a `yield` answer collapses. In real work, generators are how you stream data
too big for memory: reading a multi-GB log or rosbag (a recorded robot-sensor log)
frame by frame, or a PyTorch `IterableDataset` yielding samples on demand.

### `first_matching`

Return the first element satisfying a predicate, or a default if none does —
without consuming the iterable past the match.

```python
next((x for x in [1, 3, 4, 6] if x % 2 == 0), -1)   # -> 4
next((x for x in [1, 3] if x > 10), -1)             # -> -1
```

Where you'll see it: the "linear scan with early exit" archetype, and the inner
loop of many two-pointer problems (two indexes walking an array toward each other)
and interval-scheduling problems. The test suite uses a spy predicate
to prove yours short-circuits — the laziness is the point. In practice: find the
first frame above a threshold, the first free slot, the first config that matches —
without touching the rest of a possibly huge stream.

### `adjacent_pairs`

Return consecutive pairs: `[1, 2, 3] -> [(1, 2), (2, 3)]`; lists shorter than 2
give `[]`.

```python
lst = [1, 2, 3]
list(zip(lst, lst[1:]))   # -> [(1, 2), (2, 3)]
```

Where you'll see it: consecutive-difference problems — "Best Time to Buy and Sell
Stock II" (sum the positive consecutive deltas) and every "is it sorted / max gap
between neighbors" check: `all(a <= b for a, b in zip(lst, lst[1:]))`. In robotics
this is `dt` between consecutive timestamps and velocity from consecutive
positions: `[b - a for a, b in zip(pos, pos[1:])]` (verified: `[3, 7, 2, 8]` gives
`[4, -5, 6]`).

### `all_products`

Return every `(color, size)` combination — the Cartesian product, first argument
varying slowest.

```python
list(itertools.product(["red", "blue"], ["S", "M"]))
# -> [('red', 'S'), ('red', 'M'), ('blue', 'S'), ('blue', 'M')]
```

Where you'll see it: "Letter Combinations of a Phone Number" is a Cartesian product
over letter groups, and `product(range(-1, 2), repeat=2)` generates the 9 offsets
around a grid cell (drop `(0, 0)` to get the 8 neighbors) — the exact move "Game of
Life" and minesweeper-style grid problems need. In ML work this IS hyperparameter
grid search — trying every combination of tuning settings such as learning rate and
batch size: `product(learning_rates, batch_sizes)` enumerates every run.

## How to practice

Write your attempts in `starter.py`, then run the tests against them:

```bash
PRACTICE=1 uv run pytest ramp_up/python/02_comprehensions_and_generators -v
```

Drop the `PRACTICE=1` to run the tests against `solution.py` instead.
