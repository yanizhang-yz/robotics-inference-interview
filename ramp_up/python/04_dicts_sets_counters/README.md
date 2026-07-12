# 04 — Dicts, Sets, and Counters

## What this covers

The data-structure workhorses of Python interviews: `dict`, `set`,
`collections.Counter`, and `collections.defaultdict`. Grouping, counting,
top-k, inverting, merging, deduplicating — the patterns that make up half of
every "parse this stream of events" question.

## Why this trips up Java developers

- **You know the patterns; the ceremony is gone.** `computeIfAbsent`,
  `getOrDefault`, `merge(k, 1, Integer::sum)`, `containsKey` — each has a
  one-token Python replacement (`defaultdict`, `.get(k, default)`,
  `Counter`, `in`). Writing the Java shape in Python works but reads as an
  accent; interviewers notice.
- **Python dicts preserve insertion order** (guaranteed since 3.7). Your
  `HashMap` instincts say "order is random"; in Python every `dict` behaves
  like a `LinkedHashMap`. This enables tricks like `dict.fromkeys(items)` for
  order-preserving dedup — no `LinkedHashSet` import needed.
- **`Counter` is a dict subclass with batteries**: construct from any
  iterable, `most_common(k)` for top-k (no PriorityQueue + comparator), and
  arithmetic between counters.
- **Sets have operators**: `&`, `|`, `-`, and `a.isdisjoint(b)` replace
  `retainAll`/`containsAll` copy-then-mutate dances.
- **No `NullPointerException` chains**: `d.get(k)` returns `None` instead of
  throwing, which makes safe navigation through nested dicts a short loop
  rather than a pyramid of null checks.

Run the tests against the reference solutions:

```
uv run pytest ramp_up/python/04_dicts_sets_counters -v
```

Or write your own attempt in `starter.py` and test that instead:

```
PRACTICE=1 uv run pytest ramp_up/python/04_dicts_sets_counters -v
```

## Drills

| Drill | Idiom it teaches | Java equivalent |
|---|---|---|
| `group_by_first_letter` | `defaultdict(list)` — missing keys appear on demand | `map.computeIfAbsent(k, x -> new ArrayList<>()).add(w)` |
| `count_items` | `Counter(iterable)` | `HashMap` + `merge(item, 1, Integer::sum)` |
| `top_k_frequent` | `Counter.most_common(k)` | count map + `PriorityQueue` with comparator |
| `invert_mapping` | dict comprehension `{v: k for k, v in d.items()}` | loop over `entrySet()` into a new `HashMap` |
| `merge_sum` | `Counter` update-arithmetic (or `.get(k, 0)` loop) | `merge(k, v, Integer::sum)` per entry |
| `get_nested` | walk with `in` checks, early return — no NPEs | nested null checks / `Optional.map` chains |
| `first_duplicate` | seen-`set` scan with `in` (O(1) membership) | `HashSet` + `if (!set.add(x))` |
| `have_common_element` | `set(a).isdisjoint(b)` / `&` intersection | copy a set, `retainAll`, check `isEmpty` |
| `unique_in_order` | `dict.fromkeys(items)` — insertion order preserved! | `new LinkedHashSet<>(list)` |
| `first_unique_char` | `Counter` + `enumerate` scan, `-1` sentinel | count array/map + second indexed for-loop |
