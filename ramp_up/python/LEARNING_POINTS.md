# A Working Programmer's Map to Python

Read this **before** doing any drills — it pairs with the drill sets in this directory
(`01_*` onward). It is a reference card, not a textbook: it assumes you already program in
*some* language (variables, loops, functions, classes — nothing more specific) and shows
the Pythonic way to do each familiar thing. Come back to it whenever a drill feels weird —
the weirdness is almost always one of the items below.

---

## 1. Mental-model shifts

### 1.1 Everything is an object — including functions, classes, and `int`

Some languages split the world into primitives and objects, and make you wrap a function
in an object before you can pass it around. Python has no primitives, and functions are
ordinary values.

```python
# A function IS a value
def square(x: int) -> int:
    return x * x

f = square          # no () — the function object itself
y = f(5)
(3).bit_length()    # even int literals are objects with methods
```

### 1.2 Duck typing vs interfaces

Nominal typing asks "what type are you?" — a type must declare the contracts it satisfies.
Python asks "what can you do?" — if it has the right methods, it works. No declaration
needed.

```python
# Just define __iter__ — now `for joint in robot:` works.
class Robot:
    def __iter__(self):
        return iter(self.joints)
```

When you *want* a checkable contract (rare in interviews), `typing.Protocol` is the
modern equivalent of an interface — structural, not declared at the class.

### 1.3 No compile step — errors surface at runtime, on the line executed

A compiler catches typos, wrong arity, and missing methods before anything runs. Python
parses the file and that's it: a misspelled method inside an `if` branch explodes only
when that branch runs. Consequences for you:

- Run code early and often (`uv run pytest`, `uv run python file.py`).
- Type hints (`def f(x: int) -> str:`) are **documentation + tooling**, not enforcement.
  Nothing stops you passing a `str`. Tools like `mypy`/`pyright` check them offline — that's
  the closest thing to a compile step.

### 1.4 Indentation IS the syntax

No braces, no semicolons. A block is "everything indented one level deeper", ended by
dedenting. The colon `:` introduces every block (`if`, `for`, `def`, `class`, `with`).

```python
if x > 0:
    do_a()
    do_b()
do_c()          # dedent = block over. Mixing tabs/spaces = error. Use 4 spaces.
```

### 1.5 Modules and packages

In some languages the unit of code is the class, and file names must match class names.
In Python the unit is the **module = one `.py` file**. Any file is importable; a directory
of modules is a package. There is no "one public class per file" rule — a module freely
mixes functions, classes, and constants.

```python
import heapq                     # import a module; use as heapq.heappush(...)
from collections import deque    # pull one name in
```

`if __name__ == "__main__":` is the entry-point idiom — the block runs only when the file
is executed directly, not when imported.

### 1.6 "There should be one obvious way to do it"

Many language cultures tolerate several equivalent styles (index loop vs iterator vs
pipeline). Python culture has strong opinions: interviewers **notice** when you write the
non-idiomatic version. `for i in range(len(xs))` where `for x in xs` works reads like an
accent. The drills in this directory exist to retrain those defaults. Run `import this`
for the full manifesto ("The Zen of Python").

---

## 2. The 15 gotchas that bite newcomers hardest

### Gotcha 1: Mutable default arguments (the classic)

Default values are evaluated **once at function definition**, not per call — probably not
what your mental model of "default parameter" expects.

```python
def add_reading(reading, log=[]):      # BUG: one shared list for ALL calls
    log.append(reading)
    return log

add_reading(1)   # [1]
add_reading(2)   # [1, 2]  ← surprise, same list

def add_reading(reading, log=None):    # FIX: the None-sentinel idiom
    if log is None:
        log = []
    log.append(reading)
    return log
```

### Gotcha 2: `is` vs `==`

`==` is **value** equality (calls `__eq__`); `is` is **identity** (same object in memory).
Some languages use `==` for reference identity and a method for value equality — Python is
the other way around.

```python
[1, 2] == [1, 2]     # True  — value equality
[1, 2] is [1, 2]     # False — two distinct objects
x is None            # the ONE place you use `is`: None checks
```

Never use `is` for numbers or strings — small-int caching makes `a is b` *sometimes* true,
which is worse than always false.

### Gotcha 3: Truthiness — empty collections are false

No explicit is-empty checks needed. Empty containers, `0`, `""`, and `None` are all falsy.

```python
if queue:            # non-empty → truthy
    ...
if s:                # covers both None and ""
    ...
```

Caution: `if x:` treats `0` as false. If `0` is a valid value, write `if x is not None:`.

### Gotcha 4: `/` is float division; `//` is integer division

In many languages integer division truncates: `7 / 2 == 3`. In Python, `/` **always**
returns a float.

```python
7 / 2      # 3.5   (float, always)
7 // 2     # 3     (floor division — the integer-division operator)
-7 // 2    # -4    ← floors toward -inf, NOT truncation! Truncating languages give -3.
-7 % 2     # 1     ← the % sign follows the divisor; truncating languages give -1
```

The negative-number behavior is a real interview trap (binary search mid, modular hashing).
For truncation toward zero use `int(-7 / 2)` → `-3`.

### Gotcha 5: No `++` / `--`

There is no `++` operator: `x++` is a syntax error, and `--x` silently double-negates (it
parses as `-(-x)`!). Use `x += 1`. There is also no three-clause `for (init; test; step)`
loop — use `for i in range(n)` or `while True:`.

### Gotcha 6: Chained comparisons are real (and idiomatic)

The bounds check that takes two comparisons joined by "and" elsewhere is one chained
expression:

```python
if 0 <= i < n:       # exactly what it looks like; evaluates i once
    ...
```

Flip side: `if a == b == c:` means all-three-equal, and `x < y > z` is legal but confusing.

### Gotcha 7: No block scope — LEGB scoping

Many languages scope a variable to its enclosing braces. Python has only four scopes:
**L**ocal (function), **E**nclosing (outer function), **G**lobal (module), **B**uilt-in.
Loop variables and `if`-branch variables leak into the whole function:

```python
for i in range(10):
    last = i
print(i, last)       # 9 9 — both still alive after the loop
```

Assignment inside a function makes a name local for the *entire* function — even before the
assignment line — so reading a module-level variable then assigning it needs `global`
(or `nonlocal` for enclosing-function variables). In interviews you rarely need either;
if you do, say why.

### Gotcha 8: Late-binding closures in loops

Lambdas capture the **variable**, not its current value (some languages force captured
variables to be effectively final precisely to prevent this confusion).

```python
fns = [lambda: i for i in range(3)]
[f() for f in fns]                        # [2, 2, 2]  — all see final i

fns = [lambda i=i: i for i in range(3)]   # FIX: default arg freezes the value
[f() for f in fns]                        # [0, 1, 2]
```

### Gotcha 9: Shallow vs deep copy

`list(xs)`, `xs[:]`, `xs.copy()`, `dict(d)` all copy **one level** — nested objects are
shared (the classic trap of copying an array of references: the references are copied,
the objects are not).

```python
grid = [[0] * 3 for _ in range(3)]   # RIGHT: 3 independent rows
grid = [[0] * 3] * 3                 # WRONG: 3 references to ONE row
grid[0][0] = 1                       # ...changes "all three rows"

import copy
b = copy.deepcopy(a)                 # true deep copy when you need it
```

Also: `b = a` copies **nothing** — it's just a second name for the same object.

### Gotcha 10: Strings are immutable — `+=` in a loop is O(n²)

Each `+=` allocates a brand-new string, so repeated concatenation is quadratic — the same
reason other ecosystems have a dedicated string-builder type. Python's string builder is
"collect into a list, `''.join()` at the end":

```python
return "".join(parts)                          # one line, O(n)
return "".join(f(x) for x in xs)               # with transformation
```

Indexing is `s[i]`, and there is no separate character type: `s[i]` is a length-1 `str`.

### Gotcha 11: Exceptions as control flow — EAFP over LBYL

Many codebases check before acting ("Look Before You Leap"). Python culture prefers
"Easier to Ask Forgiveness than Permission" — try it and catch. Exceptions are cheap-ish
and idiomatic:

```python
v = d.get(k, default)        # the dict-specific answer

try:                          # the general EAFP pattern
    v = d[k]
except KeyError:
    v = default
```

`StopIteration` ends every `for` loop internally; `KeyError`/`IndexError`/`ValueError` are
normal vocabulary, not exceptional failures. Never write `except:` bare — catch specifically.

### Gotcha 12: The GIL — threads don't parallelize CPU work

CPython's Global Interpreter Lock lets only one thread execute Python bytecode at a time.
`threading` gives concurrency for **I/O-bound** work (camera reads, network, disk) but
**zero speedup for CPU-bound** work — adding threads will not make computation faster.

- CPU-bound → `multiprocessing` (separate processes), or push the work into C/NumPy
  (NumPy releases the GIL inside its kernels — this is why vectorized code parallelizes).
- I/O-bound → `threading` or `asyncio` are fine.

This comes up constantly in robotics-inference interviews ("why is your multi-threaded
preprocessing not faster?"). Know it cold. (Python 3.13+ has an experimental no-GIL build;
assume the GIL exists unless told otherwise.)

### Gotcha 13: `range` / `zip` / `map` / `dict.keys()` are lazy, not lists

They return one-shot iterators/views — lazy streams, not materialized lists:

```python
pairs = zip(names, scores)
list(pairs)      # [...all pairs...]
list(pairs)      # []  ← EXHAUSTED. Second consumption is empty, silently.
```

You can't index them (`range` is the exception — it supports `[i]` and `len`), and you
materialize with `list(...)` when you need to reuse or index. Also `map`/`filter` exist but
comprehensions are preferred (see §4).

### Gotcha 14: Negative indexing and slice forgiveness

`xs[-1]` is the last element — no length-minus-one arithmetic. `xs[-2]` second-to-last.
Out-of-range **indexing** raises `IndexError`, but **slicing** never does:
`xs[2:999]` and `"abc"[5:]` quietly clamp/return empty. That silent clamping hides
off-by-one bugs — a wrong slice bound won't throw the way a wrong index would.

### Gotcha 15: Tuple vs list — immutability is a feature you'll use

Not just "immutable list". Tuples are hashable → usable as dict keys and set members,
with no hand-written wrapper class carrying equality and hash methods:

```python
visited = set()
visited.add((row, col))            # coordinates as a set member — free
dist = {(0, 0): 0}                 # tuple key, no wrapper class needed
x, y = point                       # unpacking works on any iterable
(x,)                               # 1-tuple needs the comma; (x) is just x
```

Lists can't go in sets (`unhashable type`). Rule of thumb: tuple = fixed-shape record,
list = homogeneous growable sequence.

---

## 3. Rosetta table: concept → how Python does it

| Concept | Python | Notes |
|---|---|---|
| Resizable array | `list` | `append`/`pop` O(1) at the END; `pop(0)`/`insert(0, x)` are O(n) — use `deque` |
| Hash map | `dict` | Insertion-ordered since 3.7. `d.get(k, default)`, `d.setdefault(k, [])` |
| Hash set | `set` | Literals: `{1, 2}`; empty set is `set()` (`{}` is an empty **dict**) |
| Priority queue | `heapq` on a plain list | Min-heap only; functions not methods: `heapq.heappush(h, x)`. Max-heap: push negated values. Custom priority: push `(priority, item)` tuples |
| Sorted map / sorted set | sorted `list` + `bisect` | No built-in balanced BST. `bisect.insort` is O(n) insert; say "there's no sorted container in the standard library, so I'd keep a sorted list with bisect, or a heap if I only need the min" |
| Double-ended queue | `collections.deque` | O(1) `appendleft`/`popleft` — the BFS queue |
| Custom sort order | `key=` function | `sorted(xs, key=lambda p: (p.dist, p.name))` — return a tuple for multi-key. Descending: `reverse=True` or negate in the key. No comparator objects |
| Map/filter pipeline | comprehensions / generator expressions | `[f(x) for x in xs if p(x)]` is filter + map in one expression. Lazy version: `(f(x) for x in xs)` |
| Optional / "no value" | `None` + or-default | `value = maybe or default` (careful: treats `0`/`""` as missing); explicit: `x if x is not None else default`. Check with `is None` |
| Scoped resource cleanup | `with` statement | `with open(path) as f:` — closes on exit, exception or not |
| Interface / contract | duck typing / `typing.Protocol` | Usually you just... call the method. `abc.ABC` for enforced abstract bases |
| String builder | `"".join(parts)` | Or f-strings for formatting single values |
| Sorting | `xs.sort()` (in place) / `sorted(xs)` (new list) | Both stable (Timsort), O(n log n). `sorted` works on ANY iterable |
| Runtime type check | `isinstance(x, T)` | Accepts a tuple of types: `isinstance(x, (int, float))` |
| Printable representation | `__repr__` (debugging) / `__str__` (display) | Define `__repr__` at minimum; `print(f"{obj!r}")` calls it |
| Value equality + hashing | `__eq__` + `__hash__` | Must stay consistent with each other. Or let `@dataclass(frozen=True)` generate both |
| Data class / record | `@dataclass` | Gives `__init__`, `__repr__`, `__eq__`; `frozen=True` adds immutability + hashability |
| Doc comments | docstrings | `"""Triple-quoted, first statement in def/class."""` — runtime-accessible via `help()` |
| Build tool + dependency manager | `uv` (modern) / `pip` | `pyproject.toml` declares the project; `uv run pytest` runs in the project venv; `uv.lock` pins versions |
| Free-standing function | module-level function | Don't wrap functions in classes; a bare function in a module is idiomatic |
| Enumerated constants | `enum.Enum` | Or module-level constants for simple cases |
| Lock / mutex | `threading.Lock` | Exists and works — but see Gotcha 12 before reaching for threads |

---

## 4. Interview-speed idioms to memorize

Muscle memory. If you have to think about these, do more drills.

```python
# Swap — no temp variable
a, b = b, a

# Multiple assignment / unpacking (any iterable; * takes the rest)
x, y = point
first, *rest = xs
head, *_, tail = xs

# enumerate — NEVER for i in range(len(xs)) just to read xs[i]
for i, x in enumerate(xs):          # start=1 for 1-based
    ...

# zip — parallel iteration; stops at the shorter input
for name, score in zip(names, scores):
    ...
dict(zip(keys, values))             # two lists → dict
for a, b in zip(xs, xs[1:]):        # adjacent pairs
    ...

# Comprehensions — filter + map in one expression
squares = [x * x for x in xs if x > 0]        # list
seen    = {x % 10 for x in xs}                # set
index   = {v: i for i, v in enumerate(xs)}    # dict
total   = sum(x * x for x in xs)              # generator: no intermediate list

# f-strings — the only string formatting worth knowing
f"{name}: {score:.2f}"       # format specs after :
f"{x=}"                      # debug form → "x=42"

# collections — the three you name-drop
from collections import Counter, defaultdict, deque
Counter("mississippi").most_common(2)   # [('i', 4), ('s', 4)] — freq map in one call
Counter(a) == Counter(b)                # anagram check
g = defaultdict(list); g[u].append(v)   # adjacency list, no key-check dance
q = deque([start]); q.popleft()         # BFS queue

# Sorting with key tuples (primary key, then tie-break)
xs.sort(key=lambda w: (-counts[w], w))  # count desc, then alphabetical asc

# min / max with key
closest = min(points, key=lambda p: p[0] ** 2 + p[1] ** 2)

# any / all — short-circuiting predicates over iterables
if any(v in seen for v in neighbors): ...
if all(x > 0 for x in xs): ...

# Slicing patterns
xs[::-1]        # reversed copy (also: reversed(xs) for a lazy iterator)
xs[:k]          # first k        xs[-k:]  # last k
xs[::2]         # every other    xs[:]    # shallow copy
s == s[::-1]    # palindrome check

# Misc speed
n, m = len(grid), len(grid[0])
INF = float("inf")
for dr, dc in ((0, 1), (0, -1), (1, 0), (-1, 0)):   # 4-directional moves
    ...
divmod(seconds, 60)             # (quotient, remainder) in one call
```

---

## 5. What to say out loud in interviews

Naming the idiom signals fluency faster than anything else. Steal these lines:

- **"I'll use a dict here — Python dicts are insertion-ordered, and I'll use `.get` with a
  default rather than checking membership first."** (Shows you know EAFP and post-3.7 dicts.)
- **"For a priority queue, Python's answer is `heapq` — a min-heap over a plain list, so
  for max-heap behavior I'll push negated priorities."**
- **"There's no sorted-map container in the standard library, so I'll keep a sorted list
  with `bisect` — and I'll flag that insertion is O(n), so if inserts dominate I'd rethink."**
- **"I'll build the string parts in a list and `join` at the end — string concatenation in
  a loop is quadratic, because each `+=` copies the whole string."**
- **"Tuples are hashable, so I can put `(row, col)` straight into the visited set — no
  wrapper class needed."**
- **"I'll sort with a key tuple: negative count for descending, then the word itself for the
  tie-break — one tuple expresses the primary key and the tie-break."**
- **"Default argument gotcha — I'll take `None` and create the list inside, since defaults
  are evaluated once at definition time."**
- **"This is CPU-bound, so threads won't help because of the GIL — I'd use multiprocessing,
  or better, vectorize it in NumPy, which releases the GIL in its kernels."** (The robotics-
  inference money quote.)
- **"`zip` gives me a lazy iterator, so I'll materialize with `list` if I need two passes."**
- **"I'm using `is None`, not `== None` — identity check is the convention for None."**
- Narrate comprehensions as transformations: **"filter then map, as a list comprehension."**

Anti-signals to avoid: `for i in range(len(xs))` when you never need `i`; getters/setters
(use plain attributes); `if len(xs) > 0` (just `if xs:`); wrapping everything in a class;
`camelCase` (Python is `snake_case` for functions/variables, `PascalCase` for classes).

---

*Next step: start with drill folder `01_*` and work in order. Run any drill against your own
attempt with:* `PRACTICE=1 uv run pytest ramp_up/python/<NN_topic> -v`
