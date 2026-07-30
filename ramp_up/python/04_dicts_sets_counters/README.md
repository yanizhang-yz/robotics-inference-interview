# 04 — Dicts, Sets, and Counters

After this lesson you will be able to group, count, rank, invert, merge, and
deduplicate data in Python without writing a single hand-rolled bookkeeping
loop — and you will know which of these moves interviewers watch for. This is
the highest-payoff file in the ramp-up: roughly half of all "parse this stream
of events" interview questions are some combination of the ten drills below.

Deep dives on surrounding language rules (truthiness, `is` vs `==`, shallow
copy) live in [`../LEARNING_POINTS.md`](../LEARNING_POINTS.md); this lesson
stands alone for everything dict/set-shaped.

## The problem this lesson solves

Grouping, counting, top-k ranking, duplicate detection, ordered dedup — each
*can* be written as a hash-map loop with explicit check-then-insert
bookkeeping, an extra heap for the ranking, and a defensive copy before any
set algebra. Every one of those loops collapses to one line of Python by the
end of this lesson, because the standard library ships the bookkeeping
prebuilt: `defaultdict` for grouping, `Counter` for counting and ranking,
set operators for the algebra, and dict ordering guarantees for the dedup.

## The lesson

### 1. `dict`: a hash map with the ceremony deleted

A Python `dict` is a hash table, the classic hash-map machinery: keys go
through a **hash function** (a function turning any value into an integer) to
pick a bucket, so lookup, insert, and delete are O(1) on average. When two
keys land in the same bucket — a **hash collision** — Python probes for the
next free slot; you never handle this yourself, it only means "O(1)" is
*amortized* (fast on average, occasionally slower when the table resizes).

The everyday operations, with zero ceremony — no constructors, no method
names to remember for get/put/contains:

```python
d = {"a": 1}                     # literal syntax; no types, no new
d["b"] = 2                       # insert or overwrite
d["z"]                           # -> KeyError: 'z'   (raises, never null!)
d.get("z")                       # -> None            (the "safe" get)
d.get("z", 0)                    # -> 0               (get with a fallback)
"a" in d                         # -> True            (membership test)
1 in d                           # -> False           (`in` checks KEYS only)
for k, v in d.items():           # entry loop; k, v = unpacking
    use(k, v)
```

Three things to internalize:

- **`d[key]` on a missing key raises `KeyError`** — an exception, not a
  silent null: loud and immediate instead of a null-reference crash three
  calls later. When "missing" is a normal case, use `d.get(key, default)`.
- **`for k, v in d.items()`** iterates entries directly. The `k, v =` part is
  **unpacking**: Python splits a pair into two variables in one step — no
  entry object, no accessor calls.
- **`None` is Python's `null`**, but `d.get(k)` returning `None` never
  explodes by itself — test it with `if x is None:` and move on.

**Insertion order is guaranteed.** Since Python 3.7, every `dict` remembers
the order keys were *first* inserted and iterates in that order — every
Python dict is an insertion-ordered map, for free:

```python
d = {}
d["zebra"] = 1; d["apple"] = 2; d["mango"] = 3
list(d)                          # -> ['zebra', 'apple', 'mango']  (always)
```

Hash maps in most languages make no such promise ("iteration order is
unpredictable") — retire that instinct here. This guarantee is what makes
the ordered-dedup trick in drill 9 work.

### 2. Missing keys: the three-tier insert-if-absent ladder

The number-one unidiomatic accent interviewers notice is manual
check-then-insert:

```python
# works, but reads as translated from another language:
if key not in groups:
    groups[key] = []
groups[key].append(value)
```

The idiomatic ladder, weakest to strongest:

```python
d.get(key, 0)                        # read with a fallback, no insert
d.setdefault(key, []).append(v)      # insert-if-absent AND return the value
groups = defaultdict(list)           # auto-creates every missing entry
groups[key].append(v)                # missing key -> a fresh [] appears
```

`collections.defaultdict(list)` takes a *factory* — a zero-argument callable
(here the `list` type itself, which called with no args builds `[]`). Any
lookup of a missing key calls the factory, stores the result, and returns it.
`defaultdict(int)` gives you a counting map, because `int()` is `0`.

**The gotcha, spelled out:** the auto-create fires on *reads* too. If you
return a `defaultdict` from your function and a caller innocently does
`result["typo"]`, they get `[]` back instead of a `KeyError`, and the dict
silently grew a garbage entry:

```python
dd = defaultdict(list)
_ = dd["oops"]                   # just LOOKING added a key
dict(dd)                         # -> {'oops': []}
```

So: build with `defaultdict`, but return `dict(groups)` so normal `KeyError`
behavior comes back at the boundary. The tests in this drill check
`type(result) is dict` for exactly this reason.

### 3. `Counter`: the counting map as a built-in

`collections.Counter` is a subclass of `dict` (so everything above applies)
specialized for counting. Feed it any **iterable** — anything a `for` loop
can walk: a list, a string, a generator (a lazily-produced stream of values;
lesson 02) — and the entire read-count-increment-store loop you'd otherwise
write by hand happens in the constructor:

```python
from collections import Counter
c = Counter("mississippi")       # -> Counter({'i': 4, 's': 4, 'p': 2, 'm': 1})
c["i"]                           # -> 4
c["z"]                           # -> 0   (missing key = 0, NOT KeyError)
c.most_common(2)                 # -> [('i', 4), ('s', 4)]
```

- **Missing keys read as `0`** — no fallback-read bookkeeping anywhere.
- **`most_common(k)`** returns `(item, count)` pairs sorted by count,
  descending; ties keep first-insertion order (that dict guarantee again), so
  `Counter(["y","x","y","x"]).most_common()` is `[('y', 2), ('x', 2)]`. No
  argument = everything, sorted. This one method replaces the whole
  count-map-plus-heap-plus-comparator dance.
- **Counters do arithmetic**: `Counter("aabbb") - Counter("ab")` is
  `Counter({'b': 2, 'a': 1})`.

**Two update gotchas, spelled out:**

- `dict.update` **replaces** values: `{"b": 2}` updated with `{"b": 3}` holds
  `3`. `Counter.update` **adds**: it holds `5`. Merge two count dicts with
  plain `dict.update` and you silently lose the first dict's counts.
- `Counter(d1) + Counter(d2)` also add-merges, **but `+` drops any key whose
  total is zero or negative** (Counter arithmetic models multisets —
  collections where membership means "count > 0"): `Counter({"x": 2}) +
  Counter({"x": -2})` has no `"x"` at all, while `update()` keeps `x: 0`. For
  general number-merging, use `update()`.

### 4. `set`: the membership hash table, with operators

A Python `set` is a hash table storing only keys — no values attached.
Membership (`x in s`) is O(1); the same test on a *list* scans every element
(O(n)) — measured here, ~10,000x slower at 100k elements. Saying "I'll build
a set for O(1) membership" out loud is an interview checkbox.

```python
s = {3, 1, 2}                    # literal
s = set()                        # EMPTY set — see gotcha below
s.add(4); s.discard(99)          # discard never throws; remove() does
2 in s                           # -> True
{1, 2, 3} & {2, 3, 4}            # -> {2, 3}      intersection
{1, 2, 3} | {3, 4}               # -> {1, 2, 3, 4} union
{1, 2, 3} - {2}                  # -> {1, 3}      difference
{1, 2, 3}.isdisjoint([3])        # -> False       ("do we overlap?")
```

The operators build a **new** set and mutate nothing (in many languages the
set-algebra methods mutate the receiver instead) — so there is no defensive
copying before you dare call them.

**Gotchas, spelled out:**

- **`{}` is an empty *dict*, not an empty set.** The literal was taken by
  dicts first. Write `set()`. (`type({})` really is `dict` — check it.)
- **Sets are unordered.** `set("mississippi")` prints in arbitrary order.
  Small integers often *look* sorted (their hash is themselves) — never rely
  on it. Need order? That's drill 9's `dict.fromkeys` trick.
- **Keys must be hashable.** **Hashable** means the value can be fed to the
  hash function and will never change afterwards — in practice: immutable.
  `int`, `str`, and tuples of hashables are hashable; `list`, `dict`, and
  `set` are not. `{[1, 2]: "x"}` raises `TypeError: unhashable type: 'list'`.
  The fix is always the same: **use a tuple** — `{(1, 2): "cell"}` works, and
  `(row, col)` tuple keys replace nested maps and hand-rolled composite-key
  classes.

### 5. Dict comprehensions: reshape a map in one expression

A **comprehension** is Python's inline build-a-collection expression (lesson
02 covers them fully); the dict flavor is `{key_expr: value_expr for ... in
...}`. The loop that copies or transforms one map into another, entry by
entry, becomes a single expression:

```python
{v: k for k, v in d.items()}         # invert:  {"a": 1} -> {1: 'a'}
{k: v * 2 for k, v in d.items()}     # transform values
{k: v for k, v in d.items() if v > 0}  # filter entries
```

### 6. Mutating while iterating is an error

Modifying a collection while iterating over it *invalidates* the iterator —
its internal bookkeeping no longer matches the collection. Nearly every
language forbids it one way or another; Python refuses loudly:

```python
d = {"a": 1, "b": 2}
for k in d:
    del d[k]                     # -> RuntimeError: dictionary changed size
                                 #    during iteration
```

The fix: iterate over a **snapshot** of the keys, then mutate the real dict
freely:

```python
for k in list(d):                # list(d) copies the keys up front
    if d[k] > 1:
        del d[k]
```

Related: `d.keys()`, `d.values()`, `d.items()` return **views** — live
windows onto the dict, not copies. They even support set operators:
`d1.keys() & d2.keys()` gives the shared keys directly.

### 7. Sorting a dict (no sorted-map type needed)

Dicts don't sort themselves; you sort the *entries* and rebuild if needed:

```python
d = {"a": 3, "b": 1, "c": 2}
sorted(d.items(), key=lambda kv: kv[1], reverse=True)
# -> [('a', 3), ('c', 2), ('b', 1)]      by value, descending
```

`key=` takes a function computing the sort key — called once per element,
and elements are ordered by their keys. The `lambda kv: kv[1]` is Python's
anonymous-function syntax: `lambda <args>: <return expression>` (lesson 05
covers `lambda` in full). Here each
`kv` is one `(key, value)` pair from `d.items()`, and `kv[1]` indexes into
that pair to pull out the value, so we sort by value. For "sorted by count"
you rarely need any of this: `Counter.most_common()` already did it.

## Muscle memory

Type these until they require zero thought:

```python
from collections import Counter, defaultdict

c = Counter(items); c.most_common(k)     # count + top-k in two tokens
groups = defaultdict(list); groups[key].append(v)
d.get(key, default)                       # never getOrDefault-by-hand
if key in d:                              # never d.keys().__contains__ etc.
for k, v in d.items():                    # the entrySet loop
{v: k for k, v in d.items()}              # invert / reshape a dict
seen = set()                              # ... if x in seen: ... seen.add(x)
not set(a).isdisjoint(b)                  # "do these overlap?"
list(dict.fromkeys(items))                # ordered dedup
next((x for x in xs if cond(x)), -1)      # first match or sentinel
```

(A **sentinel** is a special value meaning "not found" — the `-1` above.)

## The drills

### `group_by_first_letter`

Group words by first character into `{letter: [words...]}`, order preserved.

```python
groups = defaultdict(list)
for word in words:
    groups[word[0]].append(word)
return dict(groups)              # strip the auto-create at the boundary
# group_by_first_letter(["apple", "bat", "avocado"])
#   -> {'a': ['apple', 'avocado'], 'b': ['bat']}
```

Where you'll see it: the grouping step inside **Group Anagrams** (LeetCode
49 — same code, sorted-letters key instead of first letter) and any "bucket
the records by X" question. In robotics/ML work it is everywhere: grouping
detections by frame id, log lines by node name, samples by class label.

### `count_items`

Count occurrences of each item; return a plain dict.

```python
return dict(Counter(items))
# count_items(["a", "b", "a"]) -> {'a': 2, 'b': 1}
```

Where you'll see it: the opening move of every frequency question — **Valid
Anagram**, **Majority Element**, **Ransom Note** all start with "count one
side". In ML pipelines: class-label histograms for dataset balance checks,
token frequency for vocabulary building. Writing the manual loop instead of
`Counter` reads like reimplementing the standard library.

### `top_k_frequent`

Return the k most frequent items, most frequent first, ties by first
occurrence.

```python
return [item for item, _count in Counter(items).most_common(k)]
# top_k_frequent(["a", "a", "b", "b", "b", "c"], 2) -> ['b', 'a']
```

The `_count` name signals "deliberately unused". `most_common` handles
`k > distinct items` by just returning everything.

Where you'll see it: **Top K Frequent Elements** (LeetCode 347) and **Top K
Frequent Words** (LeetCode 692) are near-verbatim this drill; interviewers
often accept `most_common` then ask "and without the library?" — the answer
is the heap approach from lesson 07. Real-world: most frequent error codes in
telemetry, most-triggered detection classes, hottest topics on a message bus.

### `invert_mapping`

Swap keys and values (values assumed unique and hashable).

```python
return {v: k for k, v in d.items()}
# invert_mapping({"a": 1, "b": 2}) -> {1: 'a', 2: 'b'}
```

Where you'll see it: two-directional lookups — **Isomorphic Strings** and
**Word Pattern** both maintain a map and its inverse. In ML code you invert
constantly: `class_name -> id` for training becomes `id -> class_name` for
displaying predictions; token-to-id vocabularies get inverted for decoding.

### `merge_sum`

Merge two str→number dicts, summing values on collisions, mutating neither.

```python
merged = Counter(d1)             # copies d1 — inputs stay untouched
merged.update(d2)                # Counter.update ADDS (dict.update replaces!)
return dict(merged)
# merge_sum({"a": 1, "b": 2}, {"b": 3, "c": 4}) -> {'a': 1, 'b': 5, 'c': 4}
```

Remember the section-3 trap: `Counter(d1) + Counter(d2)` also works *until* a
total hits zero or below — then the key silently vanishes. `update()` keeps
it.

Where you'll see it: the combine step of any map-reduce-shaped question —
"merge word counts from k files/workers/shards". In practice this is how you
aggregate per-episode metrics across data-loader workers or merge per-camera
detection histograms. The common follow-up, "now merge k of them", is a loop
of this (cousin to **Merge k Sorted Lists** in spirit, not code).

### `get_nested`

Follow a key path down nested dicts; return `default` if any step is missing
or non-dict.

```python
current = d
for key in keys:
    if not isinstance(current, dict) or key not in current:
        return default
    current = current[key]
return current
# get_nested({"a": {"b": {"c": 42}}}, ["a", "b", "c"]) -> 42
# get_nested({"a": {}}, ["a", "b"], default=-1)        -> -1
```

The two guards replace the null-check pyramid you would otherwise write, one
level per key: `isinstance(current, dict)` (Python's runtime type test)
stops you indexing into `5`, and `key not in current` stops the `KeyError`.

Where you'll see it: less LeetCode, more real onsites — "parse this JSON
config" tasks. In robotics you walk nested config trees daily: JSON/YAML
robot configs, ROS-style parameter trees, model metadata. Every config
library reinvents this function; write it in 20 seconds.

### `first_duplicate`

Return the first item that has appeared before; `None` if all distinct.

```python
seen = set()
for item in items:
    if item in seen:
        return item
    seen.add(item)
return None                      # implicit anyway; explicit reads better
# first_duplicate([3, 1, 4, 1, 5, 9, 5]) -> 1
```

Where you'll see it: the **seen-set scan** may be the single most reused
interview pattern — **Contains Duplicate**, **Find the Duplicate Number**,
cycle detection with a visited set, and the bookkeeping half of **Two Sum**
(seen-*dict* variant). In pipelines: spotting duplicate frame ids or sample
hashes in a dataset. Typeable from muscle memory, eyes closed.

### `have_common_element`

True if the two lists share at least one element.

```python
return not set(a).isdisjoint(b)
# have_common_element([1, 2, 3], [3, 4]) -> True
```

`isdisjoint` accepts any iterable (no need to set-ify `b`), short-circuits on
the first hit, and mutates nothing — versus a copy-then-intersect-in-place
dance that destroys the copy just to ask a question. `bool(set(a) &
set(b))` also works but builds the whole intersection first.

Where you'll see it: **Intersection of Two Arrays** and any "do these
overlap?" sub-question — does this node's topic list intersect that
subscriber's, do two label sets share a class, does a requested resource set
clash with a held one. Knowing `isdisjoint` by name is a small flex.

### `unique_in_order`

Deduplicate, keeping first-occurrence order.

```python
return list(dict.fromkeys(items))
# unique_in_order([3, 1, 3, 2, 1]) -> [3, 1, 2]
```

`dict.fromkeys(items)` builds `{3: None, 1: None, 2: None}` — later
duplicates hit an existing key and change nothing, and insertion order is
guaranteed, so the keys are your deduped sequence. `list(set(items))` is the
tempting wrong answer: it dedups but scrambles order. No insertion-ordered
set type needed — every dict already *is* one.

Where you'll see it: as a subroutine ("dedupe the event stream, keep first
occurrence") and as a Python-fluency probe — interviewers ask "how do you
dedup preserving order?" to see if you know the post-3.7 idiom. Real-world:
deduplicating waypoints in a replayed trajectory, unique-ifying label
sequences without reordering.

### `first_unique_char`

Index of the first character appearing exactly once; `-1` if none.

```python
counts = Counter(s)
return next((i for i, c in enumerate(s) if counts[c] == 1), -1)
# first_unique_char("loveleetcode") -> 2
# first_unique_char("aabb")         -> -1
```

Two-pass shape: pass 1 counts everything; pass 2 rescans *in order* asking
the counts. `enumerate(s)` yields `(index, char)` pairs so you never write
`range(len(s))`; `next((...), default)` takes the first value the inner
expression produces, or hands back the `-1` sentinel if it produces nothing —
no flag variable, no break.

Where you'll see it: **First Unique Character in a String** (LeetCode 387)
verbatim; the count-then-scan two-pass structure also solves **Sort
Characters By Frequency** and half the string-frequency family. The
`next((...), default)` idiom generalizes to any "first element matching a
predicate" search over logs, detections, or events.

## How to practice

Run the tests against the reference solutions:

```
uv run pytest ramp_up/python/04_dicts_sets_counters -v
```

Or write your own attempt in `starter.py` and test that instead:

```
PRACTICE=1 uv run pytest ramp_up/python/04_dicts_sets_counters -v
```

Aim to rewrite all ten from a blank `starter.py` in under fifteen minutes;
every one of them is a warm-up-speed expectation in a Python interview.
