# 01 — Collections and Slicing

**What this covers:** Python's `list` and `tuple` and the slicing syntax `lst[start:stop:step]`
— the everyday replacements for Java's arrays, `ArrayList`, `Arrays.copyOfRange`,
`Collections.reverse`, and `Comparator` chains.

## Why this trips up Java developers

- **There is one list type.** No `int[]` vs `ArrayList<Integer>` split, no autoboxing.
  `list` is heterogeneous, resizable, and literal-friendly: `[1, "two", 3.0]`.
- **Slicing returns a new list; Java utilities mutate in place.** `Collections.reverse(list)`
  and `Collections.sort(list)` change the list you pass in. Python gives you both, but the
  idiomatic default is the *copying* form: `lst[::-1]`, `sorted(lst)`. The mutating forms
  (`lst.reverse()`, `lst.sort()`) return `None` — a classic Java-refugee bug is writing
  `result = lst.sort()`.
- **Negative indices and out-of-range slices are legal.** `lst[-1]` is the last element
  (Java: `list.get(list.size() - 1)`), and `lst[5:100]` never throws — it just clamps.
  There is no `IndexOutOfBoundsException` for slices.
- **Tuple unpacking replaces the temp-variable dance.** `a, b = b, a` — no `int tmp = a;`.
- **Comparators are key functions.** Java transforms *comparisons*
  (`Comparator.comparing(...).thenComparing(...)`); Python transforms each *element* into a
  sort key, and tuples compare lexicographically for free.

### Side by side: rotate a list left by k

```java
// Java: index arithmetic or Collections.rotate (which mutates!)
List<Integer> out = new ArrayList<>(list);
Collections.rotate(out, -k);
```

```python
# Python: two slices, no mutation, k % len handles overflow
out = lst[k % len(lst):] + lst[:k % len(lst)]
```

## Drills

| Drill | Idiom it teaches | Java equivalent |
|---|---|---|
| `rotate_left` | slice-and-concatenate, `k % len` | `Collections.rotate(list, -k)` |
| `last_n` | negative slice `lst[-n:]` | `list.subList(size - n, size)` |
| `every_other` | step slicing `lst[::2]` | `for (int i = 0; i < a.length; i += 2)` |
| `reverse_copy` | `lst[::-1]` returns a copy | `Collections.reverse(list)` (mutates) |
| `swap_ends` | tuple unpacking `a, b = b, a` | swap with a `tmp` variable |
| `interleave` | `zip` stops at the shorter input | index loop over `Math.min(a.length, b.length)` |
| `sort_by_length_then_alpha` | tuple sort keys `key=lambda w: (len(w), w)` | `Comparator.comparing(...).thenComparing(...)` |
| `top_k_smallest` | `sorted(nums)[:k]`, clamped slices | sort a copy, `subList(0, k)` |
| `chunk` | slicing inside `range(0, len, size)` | nested index loops with bounds checks |
| `flatten_one_level` | nested list comprehension | `stream().flatMap(List::stream)` |

## How to practice

Write your attempts in `starter.py`, then run the tests against them:

```bash
PRACTICE=1 uv run pytest ramp_up/python/01_collections_and_slicing -v
```

Drop the `PRACTICE=1` to run the tests against `solution.py` instead.
