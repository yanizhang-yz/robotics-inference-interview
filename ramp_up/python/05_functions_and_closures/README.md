# 05 — Functions and Closures

## What this covers

Functions as plain values, closures, default/keyword arguments, `*args`/`**kwargs`,
decorators, EAFP-style error handling, and sort keys. In Java, "passing behavior"
means a functional interface (`Function<T,R>`, `Supplier<T>`, `Comparator<T>`) or an
anonymous class. In Python a function is just an object: assign it, pass it, return
it, stick it in a dict. No interface, no `.apply()`, no ceremony.

## Why this trips up Java developers

- **No functional-interface wrapper.** You call `f(x)`, never `f.apply(x)`. There is
  no `Function` vs `BiFunction` vs `Supplier` taxonomy — arity is checked at call time.
- **Closures can capture *and rebind* variables** with `nonlocal`. Java lambdas can
  only read effectively-final locals; the standard workaround (an `AtomicInteger` or
  one-element array) simply isn't needed here.
- **No method overloading.** One function name, one definition. Optional behavior
  comes from default parameter values and keyword arguments, not three overloads.
- **Default values are evaluated ONCE, at `def` time** — not per call. This is the
  single most famous Python bug for people coming from Java (details below).
- **EAFP over LBYL.** Idiomatic Python often tries the operation and catches the
  exception ("Easier to Ask Forgiveness than Permission") instead of pre-checking
  like Java's defensive `if (b != 0)` / `if (obj instanceof ...)` style.
- **Sorting uses a key function, not a Comparator.** You map each element to a
  sortable value (often a tuple) instead of writing pairwise `compare(a, b)` logic.
  There is no `Comparator.comparing(...).thenComparing(...)` chain — a tuple does that.

## The mutable-default-argument trap

This is the drill `append_to` exists for. The **broken** version looks completely
reasonable to a Java developer:

```python
def append_to(item, target=[]):   # BROKEN: [] is evaluated ONCE, at def time
    target.append(item)
    return target

append_to(1)   # [1]
append_to(2)   # [1, 2]  <- surprise! Both calls share the SAME list object
```

In Java terms: the default is not a per-call `new ArrayList<>()`, it behaves like a
**static field** initialized when the class loads. Every call that omits `target`
mutates that one shared list. The correct idiom is a `None` sentinel:

```python
def append_to(item, target=None):
    if target is None:
        target = []               # fresh list per call
    target.append(item)
    return target
```

## Drills

| Drill | Idiom it teaches | Java equivalent |
|---|---|---|
| `apply_n_times(f, n, x)` | Functions are values — just call `f(x)` in a loop | `Function<T,T> f; f.apply(...)` chained, or `Stream.iterate` |
| `make_multiplier(k)` | Returning a closure that captures `k` | Lambda capturing an effectively-final local |
| `make_counter()` | `nonlocal` — a closure that *rebinds* outer state | `AtomicInteger` / one-element array hack, or a stateful class |
| `safe_divide(a, b, default=None)` | Default parameter values + EAFP `try/except` | Method overloading + `if (b == 0)` pre-check |
| `append_to(item, target=None)` | The `None`-sentinel fix for mutable defaults | N/A — Java has no default args, so no trap (lucky you) |
| `describe_call(*args, **kwargs)` | Variadic positional AND keyword arguments | `Object... varargs` (positional only; no kwargs equivalent) |
| `compose(*funcs)` | Building new functions from functions, right-to-left | `Function.compose` / `andThen` chains |
| `memoize(f)` | Decorators + `functools.wraps` + dict cache | Manual `HashMap` cache, or Guava `Suppliers.memoize` / caching proxy |
| `call_with_retry(f, attempts)` | EAFP retry loop, re-raising the last exception | `try/catch` in a `for` loop around a `Supplier<T>` |
| `sort_by(records, *fields)` | `sorted(key=lambda r: tuple(...))` for multi-field sort | `Comparator.comparing(...).thenComparing(...)` |

## Run it

```bash
# Against the reference solutions (should pass out of the box):
uv run pytest ramp_up/python/05_functions_and_closures -v

# Against YOUR implementation in starter.py:
PRACTICE=1 uv run pytest ramp_up/python/05_functions_and_closures -v
```
