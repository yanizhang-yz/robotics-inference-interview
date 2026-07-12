# 03 — Strings and Text

## What this covers

Everyday string manipulation the Python way: splitting/joining, slicing,
`ord`/`chr` arithmetic, `collections.Counter`, and f-string format specs.
These come up constantly in interviews as warm-ups and as building blocks
inside larger problems (log parsing, tokenizing, formatting output).

## Why this trips up Java developers

- **You keep reaching for StringBuilder.** Python strings are immutable just
  like Java's, but the fix is different: build a list (or generator) of pieces
  and call `sep.join(pieces)` **once** at the end. Repeated `s += piece` in a
  loop is the Python equivalent of Java string concatenation in a loop — it
  works, but `join` is the idiom interviewers expect.
- **No `charAt`, no manual index loops.** Strings are sequences: `s[i]`,
  `s[::-1]` (reverse), `for c in s` (iterate characters). A reversed copy is a
  slice, not a `StringBuilder(...).reverse()`.
- **`String.format` → f-strings.** Format specs live inline:
  `f"{name:<10} {pct:.1%}"` replaces `String.format("%-10s %.1f%%", ...)`.
- **The standard library already counts for you.** `Counter(s)` replaces the
  whole `map.merge(c, 1, Integer::sum)` dance.
- **`s.split()` with no argument is special**: it splits on runs of any
  whitespace *and* drops empty strings — what you'd need `trim().split("\\s+")`
  for in Java.

Run the tests against the reference solutions:

```
uv run pytest ramp_up/python/03_strings_and_text -v
```

Or write your own attempt in `starter.py` and test that instead:

```
PRACTICE=1 uv run pytest ramp_up/python/03_strings_and_text -v
```

## Drills

| Drill | Idiom it teaches | Java equivalent |
|---|---|---|
| `reverse_words` | `s.split()[::-1]` + `" ".join(...)` | `trim().split("\\s+")`, `Collections.reverse`, `String.join` |
| `clean_palindrome` | filter comprehension + `casefold()` + compare to `[::-1]` | index-from-both-ends loop with `Character.isLetterOrDigit` |
| `char_frequencies` | `collections.Counter` | `HashMap` + `map.merge(c, 1, Integer::sum)` |
| `caesar_shift` | `ord`/`chr` arithmetic, `%` on negatives is non-negative | `charAt` + int casts, careful with negative `%` |
| `snake_to_camel` | `split('_')` + `str.capitalize` + `join`, star-unpacking `first, *rest` | `StringBuilder` loop with `Character.toUpperCase(w.charAt(0))` |
| `find_all_indices` | `str.find(sub, start)` loop — no regex needed | `indexOf(sub, fromIndex)` loop |
| `longest_common_prefix` | `zip(*strs)` transposes; `set(chars)` detects mismatch | nested index loops with bounds checks |
| `format_report` | f-string format specs: `:<10`, `:.1%` | `String.format("%-10s ... %.1f%%", ...)` |
| `join_nonempty` | `sep.join(p for p in parts if p)` | `StringBuilder` loop guarding empties, or streams + `Collectors.joining` |
| `is_anagram` | `Counter(a) == Counter(b)` | sort both `char[]` and `Arrays.equals` |
