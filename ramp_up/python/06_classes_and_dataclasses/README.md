# 06 — Classes and Dataclasses

## What this covers

Modeling data and behavior the Python way: `@dataclass` instead of POJO/Lombok
boilerplate, dunder methods (`__add__`, `__len__`, `__lt__`, `__repr__`) instead of
implementing interfaces, `@property` instead of getter/setter pairs, `@classmethod`
factories, and duck typing instead of declared interfaces.

## Why this trips up Java developers

- **A POJO is one decorator, not fifty lines.** `@dataclass` generates `__init__`,
  `__eq__`, and `__repr__` from the field declarations — it's Lombok's
  `@Data`/`@Value` built into the standard library. `frozen=True` gives you an
  immutable value type with a correct `__hash__` for free: the whole
  `equals()`/`hashCode()` contract you'd otherwise hand-write (and get subtly wrong).
- **Operators are methods you can define.** Java reserves `+` for numbers and
  strings, so vector math becomes `a.add(b).scale(2)` ceremony (see `BigDecimal`).
  In Python, defining `__add__` makes `a + b` work on your own type.
- **No getters/setters — until you need them.** Public attributes are idiomatic
  (`temp.celsius = 20`). When a computed or validated accessor becomes necessary,
  `@property` retrofits logic behind the *same attribute syntax*, so callers never
  change. This is why Python doesn't need the "always write getters just in case"
  rule drilled into Java developers.
- **Protocols are dunders, not interfaces.** You don't implement `Iterable<T>` or
  `Comparable<T>`; you define `__iter__` or `__lt__` and the whole language —
  `for`, `len()`, `if stack:`, `sorted()`, `heapq` — cooperates.
- **Duck typing replaces interface declarations.** A function that calls
  `obj.speak()` works with ANY object that has a `speak()` method — no
  `implements Speaker`, no type hierarchy, no cast. "If it quacks, it's a duck."
- **`__repr__` is `toString()` with a convention:** aim for output that looks like
  the constructor call (`Vector2D(x=1.0, y=2.0)`), which is what you want in logs
  and debugger views. Dataclasses generate exactly that format.

## Drills

All drills live in one module. Every class has a sensible `__repr__` — the tests
check it.

| Drill | Idiom it teaches | Java equivalent |
|---|---|---|
| `Vector2D` | `@dataclass(frozen=True)` value type; `__add__`/`__sub__` operator overloading; free `__eq__`/`__hash__` | Immutable POJO + Lombok `@Value` + hand-written `equals`/`hashCode`; `BigDecimal.add`-style method chaining |
| `Temperature` | `@property` + setter: computed attribute behind plain attribute syntax | `getFahrenheit()` / `setFahrenheit(double)` pair |
| `Stack` | Container dunders: `__len__`, `__bool__`, `__iter__`; EAFP `pop()` raising `IndexError` | Implementing `Iterable<T>`, `isEmpty()`, `size()`; `EmptyStackException` |
| `Task` | `__lt__` so `sorted()` and `heapq` order your type directly | `implements Comparable<Task>` + `compareTo` |
| `Point.from_string` | `@classmethod` factory constructor (`cls(...)`) | Static factory method `Point.fromString(String)` |
| `describe(obj)` + `Dog`, `Robot` | Duck typing: any object with `.speak()` works, no shared base | `interface Speaker { String speak(); }` + `implements` on every class |

## Run it

```bash
# Against the reference solutions (should pass out of the box):
uv run pytest ramp_up/python/06_classes_and_dataclasses -v

# Against YOUR implementation in starter.py:
PRACTICE=1 uv run pytest ramp_up/python/06_classes_and_dataclasses -v
```
