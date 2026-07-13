# 02 — Ownership and RAII: unique_ptr, std::move, destructors

This is the chapter Java never had. After this lesson you will be able to: write a class
that owns heap memory and frees it automatically with zero cleanup code, hand an object
from one function to another so that exactly one place is ever responsible for it, and
build "run this cleanup at the closing brace, no matter what" utilities — the pattern
behind every file handle, lock, and timer in production C++. Every term is defined the
first time it appears; every snippet's output was verified with `clang++ -std=c++17`.

## The Java you know

```java
// Memory: every object lives on the heap; variables are references; the GC frees things.
int[] data = new int[5];        // heap allocation, zero-initialized
int[] alias = data;             // a second reference to the SAME array — cheap, always legal
// Nobody "owns" the array. It lives until the garbage collector notices that no
// reference can reach it anymore — milliseconds or minutes later, you can't know.

// Cleanup: only AutoCloseable objects get deterministic release, and only
// inside try-with-resources:
try (FileInputStream in = new FileInputStream("cfg.bin")) {
    // ... use in ...
}                               // close() runs here, guaranteed
// Everything else gets finalize()/Cleaner: runs "eventually, maybe, on some thread".
```

Both halves of this model are different in C++: there is no garbage collector, and
*every* object gets the try-with-resources treatment automatically.

## The lesson

### 1. Ownership: the question Java never asks

Two kinds of memory first (Java has them too, it just hides one):

- The **stack** is the scratch space a function gets when it's called and gives back the
  instant it returns. Fast, automatic, but dies with the function.
- The **heap** is the big shared pool for data that must outlive a single function call.
  In Java, *all* objects live here and the **garbage collector** (GC) — background
  machinery that periodically finds unreachable objects — frees them for you.

C++ has no GC. Heap memory you allocate stays allocated until some line of code frees
it. Get that wrong and you hit the three classic C++ bugs:

- **Memory leak** — you allocated and never freed; the process's memory grows forever.
- **Dangling pointer** — you kept an address to memory that was already freed. A
  **pointer** is just a variable holding a memory address (a Java reference is a pointer
  you're not allowed to do arithmetic on). Touching freed memory through one is
  **undefined behavior** — a C++ term of art meaning the standard allows *anything*:
  a crash, silently wrong values, or code that works for months and fails in a demo.
  Java throws `NullPointerException`; C++ does not check.
- **Double free** — two places both freed the same memory. Crash, or silently
  scrambled memory.

Modern C++ prevents all three with one discipline: **ownership**. Every heap object has
*exactly one* owner at any moment. The owner — and only the owner — frees the object,
automatically, at the moment the owner itself dies. The rest of this lesson is the
machinery that makes the compiler enforce that discipline for you.

### 2. Destructors: cleanup that runs on a specific line

A **destructor** is a method named `~ClassName()` that the compiler calls automatically
at the exact moment an object dies. For a local variable, that moment is the closing
brace of its **scope** — the region between `{` and `}` where the variable exists. (The
demo below prints `ctor` from the constructor and `dtor` from the destructor — those are
just common C++ shorthand for the two words, not keywords.)

```cpp
struct Noisy {                 // struct = class whose members are public by default
    std::string n;
    Noisy(std::string name) : n(name) { std::cout << "ctor " << n << "\n"; }
    ~Noisy()                          { std::cout << "dtor " << n << "\n"; }
};

int main() {
    Noisy a("a");
    {
        Noisy b("b");
        Noisy c("c");
    }                                  // <- b and c die HERE, on this line
    std::cout << "after block\n";
}                                      // <- a dies here
// Output:
// ctor a
// ctor b
// ctor c
// dtor c        <- destructors run in REVERSE construction order
// dtor b
// after block
// dtor a
```

Two properties Java cannot give you:

- **Deterministic**: cleanup happens *on that brace*, not "when the GC feels like it".
  Java's `finalize()` runs eventually-maybe and is deprecated; `try-with-resources` is
  deterministic but only for `AutoCloseable`, and only where the caller remembers to
  write the `try`. A C++ destructor needs no interface and no cooperation from callers.
- **Exception-proof**: if an exception is thrown, destructors of everything in the
  scopes being exited still run on the way out:

```cpp
struct Guard { ~Guard() { std::cout << "cleanup ran\n"; } };
try {
    Guard g;
    throw std::runtime_error("boom");
} catch (const std::exception& e) {
    std::cout << "caught: " << e.what() << "\n";
}
// Output:
// cleanup ran      <- destructor fired DURING the exception, before the catch
// caught: boom
```

So in C++ every class is its own `finally` block.

### 3. RAII: the pattern with the terrible name

**RAII** stands for "Resource Acquisition Is Initialization" — an awful name for a
simple idea: *tie a resource to an object's lifetime*. The constructor acquires the
resource (opens the file, locks the mutex, allocates the memory); the destructor
releases it. Then scope rules from section 2 guarantee the release with no cleanup code
at the call site, ever.

The entire standard library is built on this one mechanism:

| RAII class | Constructor acquires | Destructor releases |
|---|---|---|
| `std::ifstream` | opens the file | closes it |
| `std::lock_guard` | locks a **mutex** (a lock only one thread may hold at a time) | unlocks it |
| `std::vector` | (grows) heap array for its elements | frees it |
| `std::unique_ptr` | takes charge of a heap object | deletes it |

Where Java needed three language features (`finally`, try-with-resources, `Cleaner`),
C++ needs only destructors. Deep dive: `../LEARNING_POINTS.md` §2–3.

### 4. `std::unique_ptr`: ownership as a type

A **smart pointer** is an object that *behaves* like a pointer — you can **dereference**
it (follow the address to reach the object it points at), and it can be null — but has a
destructor that frees what it points to. `std::unique_ptr<T>` is the sole-owner smart
pointer: the one place responsible for one heap object. The angle brackets are a
**template parameter** — like Java generics, except they work for any type including
`int`. (Java generics can't hold a primitive: the compiler wraps each `int` in an
`Integer` object — that wrapping is *autoboxing*, one heap allocation per value. C++
templates generate real code for `int` itself; no wrappers exist. See
`../LEARNING_POINTS.md` §9.)

You almost never write `new`, and *never* `delete`. The one-step allocator is
`std::make_unique`:

```cpp
auto p = std::make_unique<std::string>("hi"); // heap-allocate a string("hi"); p owns it
std::cout << p->size();       // -> 2      `->` reaches members through a pointer (Java's `.`)
std::string* raw = p.get();   // borrow the raw address; ownership does NOT change
if (p) { /* runs */ }         // a unique_ptr converts to true when non-null (Java: p != null)
p.reset();                    // free the string NOW; p becomes null
// (p == nullptr) -> true        nullptr is C++'s typed null literal
```

There is a second form for arrays: `std::unique_ptr<int[]>` (note the `[]`). It frees
with the array flavor of delete, gives you plain indexing `a[i]`, and
`std::make_unique<int[]>(n)` **zero-initializes** — every element starts at 0, exactly
like Java's `new int[n]`:

```cpp
auto a = std::make_unique<int[]>(5);
// a[0] + a[1] + a[2] + a[3] + a[4] -> 0   (zero-initialized)
a[0] = 7;                                // index it like a plain array
```

The crucial rule: **a `unique_ptr` cannot be copied**, because a copy would mean two
sole owners. The copy operation is deleted, so this is a *compile* error, not a runtime
surprise:

```cpp
auto p = std::make_unique<int>(1);
auto q = p;   // error: call to implicitly-deleted copy constructor of 'unique_ptr<int>'
```

And it costs nothing: `sizeof(std::unique_ptr<int>) == sizeof(int*)` (both 8 bytes on
this Mac). It's the same plain address a raw pointer holds, plus a destructor. C++ calls
this a *zero-cost abstraction*: the safety is free at runtime.

What if two places genuinely must share an object, Java-style? That exists —
`std::shared_ptr`, which keeps a count of how many pointers share the object and frees
it when the count hits zero — but it's the exception, not the default. See the pointer
taxonomy in `../LEARNING_POINTS.md` §4.

### 5. `std::move`: handing over the keys

If copying is banned, how does a `unique_ptr` ever leave the function that made it?
By **moving**. `std::move(p)` is — despite the name — just a cast: it moves nothing
itself, it only marks `p` as "you may steal from this". The actual transfer happens in
the receiving `unique_ptr`, which takes the address and nulls out the source:

```cpp
auto p = std::make_unique<int>(42);
auto q = std::move(p);   // ownership transferred to q
// p == nullptr  -> true    a moved-from unique_ptr is GUARANTEED null
// *q            -> 42
```

Java has no syntax for "I'm giving this reference away" — every Java variable is an
equally-powerful alias forever. In C++ the handoff is visible in the source, and the
compiler *forces* the visibility. Passing a `unique_ptr` to a by-value parameter without
`std::move` is the same deleted copy as before:

```cpp
void sink(std::unique_ptr<int> owned);
sink(p);              // error: call to implicitly-deleted copy constructor
sink(std::move(p));   // OK — and everyone reading this line can SEE the handoff
```

Three gotchas, each one sentence you can act on:

- **After `sink(std::move(p))`, `p` is null.** `*p` or `p->anything` still compiles but
  crashes with a **segmentation fault** — the operating system killing your process for
  touching an invalid address. (Verified: the process dies instantly — no exception, no
  stack trace.) The type system made the transfer visible; not using `p` afterwards is
  on you.
- **Never write `return std::move(local);`** — returning a local `unique_ptr` already
  moves it, and the explicit `std::move` defeats **copy elision** (the optimization
  where the compiler constructs the return value directly in the caller's variable,
  skipping even the move). Clang literally warns: *"moving a local object in a return
  statement prevents copy elision"*. Plain `return p;` is correct.
- **`std::move` doesn't move.** If nothing steals from the marked variable, nothing
  happens at all. It's a label, not an action.

Deeper treatment of moves for all types (not just pointers): `../LEARNING_POINTS.md` §10.

### 6. The rule of zero

If a class hand-writes a destructor, it usually also needs hand-written copy and move
rules — in C++ folklore, the "rule of three/five" (destructor + copy pair + move pair
travel together). The modern escape hatch is the **rule of zero**: make every field a
self-cleaning type (`std::vector`, `std::string`, `std::unique_ptr`) and write *none*
of the five. The compiler-generated destructor destroys each field, each field frees its
own resource, done. The `Buffer` drill below is a rule-of-zero class: it owns heap
memory yet contains no cleanup code whatsoever.

### 7. Syntax you'll need for the drills

- **Member initializer list** — the `: field_(value), ...` between a constructor's
  signature and its body. It *initializes* fields before the body runs (Java only has
  assignments inside the body). It is the required way to set up `const` fields and
  reference fields, and the idiomatic way for everything else:

  ```cpp
  explicit Buffer(std::size_t n) : size_(n), data_(std::make_unique<int[]>(n)) {}
  ```

- **`explicit`** on a one-argument constructor forbids the compiler from silently using
  it as a conversion: without it, `use(5)` would auto-manufacture a `Buffer(5)`; with
  it, that line is an error ("no matching function") and you must write `use(Buffer(5))`.
  Habit: mark every single-argument constructor `explicit`.
- **`const` after a method** — `std::size_t size() const` — promises the method won't
  mutate the object. Calling a non-`const` method on a `const` object/reference is a
  compile error ("function is not marked const"), so mark every read-only method
  `const` or callers taking `const Buffer&` can't use it. (`../LEARNING_POINTS.md` §5.)
- **`= delete`** removes a function at compile time. `ScopedLogger(const ScopedLogger&)
  = delete;` makes any attempted copy the error "call to deleted constructor" — exactly
  how `unique_ptr` bans copying.
- **Reference member** — `std::vector<std::string>& log_;` stores an *alias* to the
  caller's vector: borrowed, not owned, so the destructor has no duty to it. It must be
  initialized in the member initializer list, and the borrowing object must not outlive
  the thing it borrows.
- **`std::size_t`** is the unsigned (never negative) integer type C++ uses for sizes and
  indexing, where Java uses `int`. **`long long`** is a guaranteed-64-bit integer —
  Java's `long`.

## Muscle memory

Type these without thinking:

```cpp
auto p = std::make_unique<Widget>(args);      // heap-allocate; p is sole owner
auto a = std::make_unique<int[]>(n);          // owned array, zero-initialized
p->method();   (*p).method();   p.get();      // use through the pointer / borrow raw
if (p) { }     p == nullptr;                  // null checks
sink(std::move(p));                           // give ownership away; p is null after
std::unique_ptr<T> make();                    // return type says: caller receives ownership
void sink(std::unique_ptr<T> t);              // by-value param says: this function consumes it
~ClassName() { /* release */ }                // destructor = deterministic finally
ClassName(const ClassName&) = delete;         // forbid copying at compile time
explicit ClassName(std::size_t n) : field_(n) {}  // explicit ctor + initializer list
```

## The drills

Open `starter.cpp`; each stub restates its own hints in comments.

### Drill 1 — `Buffer`: a class that owns heap memory

Task: constructor stores `n` and allocates `std::unique_ptr<int[]>`; implement
`size()`, `fill(v)`, and `sum()` (as `long long`).

```cpp
explicit Buffer(std::size_t n) : size_(n), data_(std::make_unique<int[]>(n)) {}
// Buffer b(5);   b.sum() -> 0   (zero-initialized, like Java's new int[5])
// b.fill(3);     b.sum() -> 15
// b dies at its closing brace -> array freed, no code written for it (rule of zero)
```

Gotcha: `size()` and `sum()` must be marked `const` (they already are in the stub) —
delete the `(void)` placeholder lines once you use the fields for real.

**Where you'll see it:** the classic C++ interview "implement a String / dynamic array /
Matrix class" is really a resource-ownership test — the follow-up is always "what
happens when I copy it?" (with a `unique_ptr` member: it doesn't compile, which is often
the answer they want, plus how you'd add a deliberate **deep copy** — a hand-written copy
constructor that allocates a fresh array and duplicates the elements, instead of the
banned pointer copy). In robotics/ML work
this class *is* the shape of an image frame, a point-cloud buffer, or a tensor
input/output staging buffer — inference APIs hand you raw memory (often on the GPU) and
production code wraps it in exactly this kind of owning class.

### Drill 2 — `makeBuffer`: a factory that hands ownership out

Task: heap-allocate a `Buffer` and return it as `std::unique_ptr<Buffer>`.

```cpp
std::unique_ptr<Buffer> makeBuffer(std::size_t n) {
    return std::make_unique<Buffer>(n);   // plain return — NO std::move here
}
// auto buf = makeBuffer(4);   buf->size() -> 4
```

Gotcha: writing `return std::move(...)` on a local triggers the "prevents copy elision"
warning from §5 — a plain `return` of a local `unique_ptr` moves automatically.

**Where you'll see it:** factory-pattern questions, and the standard screener "why
return `unique_ptr` instead of a raw pointer?" (the signature documents that the caller
now owns it, and the object can't leak even if the caller ignores the result). In real
systems, driver and plugin factories ("give me a camera object for this config") return
`unique_ptr`; inference runtimes' create-functions return heap objects that production
code wraps in `unique_ptr` on the very next line.

### Drill 3 — `moveBuffer`: a function that consumes ownership

Task: take `std::unique_ptr<Buffer>` **by value**, return its `sum()`; the `Buffer` is
destroyed when the parameter dies at the function's end.

```cpp
auto buf = makeBuffer(3);
buf->fill(7);
long long total = moveBuffer(std::move(buf));  // handoff visible at the call site
// total -> 21
// buf == nullptr -> true   (the Buffer was destroyed INSIDE moveBuffer)
```

Gotcha: forgetting `std::move` at the call site is a compile error (deleted copy);
touching `buf`'s contents after the move is a crash. Both from §5.

**Where you'll see it:** the bread-and-butter C++ interview trio — "what does
`std::move` actually do?", "what state is a moved-from object in?", and "implement
`unique_ptr`". In robotics this "sink function" signature is everywhere: ROS 2 (the
standard robotics middleware) lets you `publish()` a `std::unique_ptr` message precisely
so the framework can pass your buffer to subscribers without copying, and pipeline
stages (capture → preprocess → inference) hand frames along the same way.

### Drill 4 — `ScopedLogger`: the RAII scope guard

Task: constructor appends `"enter"` to the borrowed log vector, destructor appends
`"exit"`. Copying is already `= delete`d in the stub.

```cpp
std::vector<std::string> log;
{
    ScopedLogger logger(log);   // log: ["enter"]
    log.push_back("work");
}                               // destructor fires AT this brace
// log -> ["enter", "work", "exit"]
```

The nested case in `main()` checks reverse destruction order from §2: two nested loggers
produce `enter, enter, exit, exit` — inner one exits first.

**Where you'll see it:** "What is RAII?" is arguably the single most common C++
interview question, and "implement `lock_guard`" / "make this exception-safe without
`finally`" are its hands-on forms — this drill is that exact exercise. In real code this
pattern is `std::lock_guard` around shared state in every multithreaded control loop,
scoped timers that log how long an inference call took, and safety guards like "stop the
motors when this scope exits, even if an exception is flying".

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
