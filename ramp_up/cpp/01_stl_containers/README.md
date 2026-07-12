# 01 — STL containers: vector, string, unordered_map, map, unordered_set

Five small drills in one file covering the C++ equivalents of `ArrayList`, `String`,
`HashMap`, `TreeMap`, and `HashSet` — plus the parameter-passing idioms
(`const&` vs by-value) that C++ interviewers watch for from line one.

## Why this trips up Java developers

- **Everything copies by default.** In Java, passing an `ArrayList` to a method hands over
  a reference; in C++, `void f(std::vector<int> v)` silently clones the whole vector.
  The read-only default is `const std::vector<int>&` — no copy, no mutation allowed.
- **`m[key]` is not `get(key)`.** On `unordered_map`/`map`, `operator[]` *inserts* a
  default-constructed value when the key is absent. Perfect for counters (`++freq[c]`),
  a footgun for lookups (use `find`/`count`/`at`).
- **`HashMap` vs `TreeMap` is `unordered_map` vs `map`** — same split, but in C++ the
  *sorted* one got the short name. `std::map` iterates in key order, O(log n) ops.
- **`set.add(x)` returning `boolean`** becomes `s.insert(x).second` — insert returns a
  `pair<iterator, bool>`.
- **Java `long` ≠ C++ `long`.** C++ `long` is 32-bit on some platforms; the guaranteed
  64-bit spelling is `long long`.
- **`Map.Entry` is gone.** Iterating a map yields pairs; C++17 structured bindings
  (`for (const auto& [key, value] : m)`) unpack them in place.

## Drills

| Drill | Idiom it teaches | Java equivalent |
|---|---|---|
| `reverseWords` | `std::istringstream` tokenizing, building a `std::string` by value, reverse iterators | `split("\\s+")` + `Collections.reverse` + `String.join` |
| `charFrequencies` | `unordered_map` counting via `operator[]` auto-insert; range-based `for` over a string's chars | `HashMap` + `merge(c, 1, Integer::sum)` |
| `topKSmallest` | take the vector **by value** when you need your own copy anyway; `std::sort` + `resize` | `new ArrayList<>(list)` + `Collections.sort` + `subList(0, k)` |
| `groupByLength` | `std::map` = sorted keys; `const auto&` loop variable; structured bindings | `TreeMap` + `computeIfAbsent(len, k -> new ArrayList<>())` |
| `sumOfUnique` | `unordered_set::insert(x).second` dedup; `long long` for 64-bit sums | `HashSet.add()` boolean + `long` accumulator |

## How to practice

Implement the TODO stubs in `starter.cpp`, then run the tests against your code:

```sh
PRACTICE=1 uv run pytest ramp_up/cpp/01_stl_containers -v
```

Or compile and run it directly — `main()` asserts every drill and prints `ALL TESTS PASSED`:

```sh
clang++ -std=c++17 -Wall -o /tmp/stl starter.cpp && /tmp/stl
```

Without `PRACTICE=1`, pytest checks the reference `solution.cpp` instead.
