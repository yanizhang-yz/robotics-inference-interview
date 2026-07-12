# 02 — Ownership and RAII: unique_ptr, std::move, destructors

The chapter Java never had: who *owns* a heap object, how ownership moves between
functions, and how destructors give you deterministic cleanup without a garbage
collector. You build a heap-owning `Buffer` class, a factory, an ownership-transfer
function, and a `ScopedLogger` — the classic RAII pattern behind `lock_guard`,
`ifstream`, and every "do X on scope exit" utility in real codebases.

## Why this trips up Java developers

- **No GC.** In Java every reference is equally powerful and the collector figures out
  lifetimes. In C++ exactly one `std::unique_ptr` owns each heap object; when that
  pointer dies, the object is freed — immediately, on that exact line.
- **`unique_ptr` cannot be copied** — copying would mean two sole owners. Handing it to
  a function or another variable requires `std::move`, which makes ownership transfer
  *visible in the source code*. Java has no syntax for "I'm giving this away."
- **Destructors are the anti-`finalize()`.** Java's finalizers run "eventually, maybe";
  `try-with-resources` bolted determinism on for `AutoCloseable` only. A C++ destructor
  runs at the closing brace, every time, exceptions included — so *any* class can be
  its own try-with-resources.
- **A moved-from `unique_ptr` is guaranteed null.** After `f(std::move(p))`, using `*p`
  is a null dereference. The compiler won't stop you — this is your first taste of
  "the type system helped, but the discipline is still yours."
- **`new`/`delete` almost never appear** in modern code. `std::make_unique<T>(args)`
  allocates and wraps in one step; nobody writes `delete` because destructors do it.

## Drills

| Drill | Idiom it teaches | Java equivalent |
|---|---|---|
| `Buffer` class | `std::unique_ptr<int[]>` member = heap array freed automatically ("rule of zero" — no hand-written destructor needed); `const` methods | `int[]` field, GC frees it eventually |
| `makeBuffer` | factory returning `std::unique_ptr` — ownership flows out of the function | static factory method returning a reference |
| `moveBuffer` | taking `unique_ptr` **by value** = consuming ownership; caller must `std::move`; callee's scope end destroys the object | passing a reference (aliases only — Java can't express handing over the *only* reference) |
| `ScopedLogger` | RAII: constructor/destructor as deterministic enter/exit hooks; dtors run in reverse construction order | `try-with-resources` / `finally`, but usable for *anything*, not just `AutoCloseable` |

## How to practice

Implement the TODO stubs in `starter.cpp`, then run the tests against your code:

```sh
PRACTICE=1 uv run pytest ramp_up/cpp/02_ownership_and_raii -v
```

Or compile and run it directly — `main()` asserts every drill and prints `ALL TESTS PASSED`:

```sh
clang++ -std=c++17 -Wall -o /tmp/raii starter.cpp && /tmp/raii
```

Without `PRACTICE=1`, pytest checks the reference `solution.cpp` instead.
