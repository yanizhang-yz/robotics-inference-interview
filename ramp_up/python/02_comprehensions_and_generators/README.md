# 02 — Comprehensions and Generators

**What this covers:** list/dict/set comprehensions, generator functions (`yield`),
generator expressions, and the `zip` / `itertools` toolkit — Python's answer to
`java.util.stream`.

## Why this trips up Java developers

- **Streams are the wrong mental model to port literally.** Python has `map()` and
  `filter()`, but chaining them is considered clunky. The idiomatic form is a
  comprehension, which reads like set-builder notation instead of a pipeline:

  ```java
  // Java
  List<Integer> out = nums.stream()
      .filter(n -> n % 2 == 0)
      .map(n -> n * n)
      .collect(Collectors.toList());
  ```

  ```python
  # Python — filter and map in one expression, no collector
  out = [n * n for n in nums if n % 2 == 0]
  ```

- **No `Collectors`.** The bracket picks the container: `[...]` list, `{k: v ...}`
  dict, `{...}` set, and plain parentheses `(...)` make a *lazy generator*.
- **Generators are the laziness story.** Java streams are lazy until a terminal
  operation; in Python, laziness lives in generators. A function containing `yield`
  returns an iterator that runs on demand — like an `Iterator<T>` you'd otherwise
  hand-write with `hasNext()`/`next()` state fields, in three lines.
- **Generators are single-use.** A Java `List` can be re-streamed forever; an exhausted
  generator yields nothing on the second pass. Wrap in `list()` when you need to reuse.
- **`findFirst().orElse(x)` is one built-in call:** `next(gen_expr, default)`.
- **`zip(*matrix)` is an idiom worth memorizing** — the `*` unpacks rows as arguments,
  and `zip` staples them back together column-wise. Transpose in one line, no
  `new int[cols][rows]` allocation dance.

## Drills

| Drill | Idiom it teaches | Java equivalent |
|---|---|---|
| `squares_of_evens` | list comprehension with `if` filter | `stream().filter().map().collect(toList())` |
| `word_length_map` | dict comprehension `{w: len(w) ...}` | `stream().collect(toMap(w -> w, String::length))` |
| `unique_first_letters` | set comprehension with a guard | `stream().filter().map().collect(toSet())` |
| `transpose` | `zip(*matrix)` argument unpacking | nested index loops into `new int[cols][rows]` |
| `flatten_matrix` | nested comprehension clause order | `stream().flatMap(List::stream)` |
| `running_totals` | `itertools.accumulate` | manual loop with a running `sum` variable |
| `countdown` | writing a generator with `yield` | implementing `Iterator<Integer>` by hand |
| `first_matching` | `next(genexpr, default)` | `stream().filter(p).findFirst().orElse(d)` |
| `adjacent_pairs` | `zip(lst, lst[1:])` self-zip | index loop `for (i = 0; i < n - 1; i++)` |
| `all_products` | `itertools.product` | nested for-loops building pairs |

## How to practice

Write your attempts in `starter.py`, then run the tests against them:

```bash
PRACTICE=1 uv run pytest ramp_up/python/02_comprehensions_and_generators -v
```

Drop the `PRACTICE=1` to run the tests against `solution.py` instead.
