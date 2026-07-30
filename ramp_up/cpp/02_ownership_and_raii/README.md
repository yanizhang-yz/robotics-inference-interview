# 02 — Ownership and RAII: unique_ptr, std::move, destructors

This lesson teaches the most important idea in C++. After it, you will be able to: write
a class that owns heap memory and frees it automatically with zero cleanup code, hand an
object from one function to another so that exactly one place is ever responsible for it,
and build "run this cleanup at the closing brace, no matter what" utilities — the pattern
behind every file handle, lock, and timer in production C++.

Every term is defined the first time it appears; every snippet's output, error message,
and exit code was verified with `clang++ -std=c++17` on this Mac.

## The problem this lesson solves

Lesson 01 taught you that a variable is a box of bytes, and that a local variable's box
vanishes at its closing brace. That model has a hole in it: what about data that must
**outlive** the function that created it? A factory function that builds an object and
returns it. A camera frame that travels through a pipeline long after the capture
function returned. That data can't live in a box that dies with the function.

Every language has to answer this. Python's answer is the **garbage collector**: every
object lives in one big pool, the runtime tracks who still refers to it, and memory
whose last reference disappears gets reclaimed for you. You wrote eight Python drill
sets without freeing a single byte — that machinery was working the whole time. It works
— at the cost of *when*. Collection runs when the runtime decides, and it can pause your
program to do it.

C++ refuses that trade, on purpose. A robot control loop at 50 Hz has 20 milliseconds
per tick, every tick, forever; an inference server promises a latency budget per request.
A collector that pauses the world at a moment of its choosing breaks both. So C++ has no
garbage collector — and that means some line of *your* code must free every piece of
long-lived memory, at a moment you can point to.

Doing that by hand is famously error-prone. This lesson shows you the failure modes
honestly, then the C++ solution — which is not "be careful", but a mechanism that makes
cleanup automatic *and* deterministic at once: ownership, enforced by destructors.

## The lesson

### 1. The stack: memory that frees itself

When a function is called, it gets a **stack frame**: a block of scratch memory for its
local variables. When the function returns, the frame is handed back — instantly, by
moving a single pointer. All the frames form the **stack**: called functions pile frames
on top, returns pop them off.

```cpp
int square(int x) {     // calling square pushes a frame; x and result live in it
    int result = x * x;
    return result;
}                       // frame popped: x and result are GONE, cost ~zero

int main() {
    int nine = square(3);   // nine lives in main's frame
}
```

Every local variable you have written so far lived on the stack. That's why lesson 01's
scope rule works: the closing brace *is* the deallocation. Stack memory is fast,
automatic, and impossible to leak — and it dies with the function. Which is exactly why
it can't hold data that must outlive the function.

### 2. The heap: memory that outlives the function

For that, C++ gives you the **heap**: a big pool of memory that any function can allocate
from, where blocks live until some line of code explicitly frees them.

**Where the heap lives.** Every running program (a *process*) gets its own private
address space from the operating system. Inside it, different regions have different
jobs:

```
+---------------------------+
|  your compiled code       |   fixed at load time
+---------------------------+
|  heap                     |   grows downward as you allocate
|    ...                    |
|    (free space)           |
|    ...                    |
|  stack                    |   grows upward as functions call functions
+---------------------------+
```

The stack and the heap are both just memory — same RAM, same addresses. What differs is
**who manages them**. The stack is managed by function calls themselves: enter a
function, its frame appears; return, it vanishes. The heap is managed by the
**allocator** — a bookkeeping library (the machinery behind `new`) that owns the heap
region and runs a ledger over it, like a hotel front desk: which rooms are occupied,
which are free.

**How you put data on the heap.** You ask the allocator with `new`:

```cpp
int* p = new int(42);   // heap-allocate an int, initialized to 42
```

`new` finds a free chunk, marks it *occupied* in the ledger, and hands back its
**address** — the room number. `p` is a **pointer**: a variable whose own 8-byte box
holds that address. Note what lives where: the int is on the heap; the pointer holding
its address is an ordinary stack variable. `*p` reads the value at the address
("dereferencing"). When the function returns, the pointer's box dies with the frame —
but the heap int lives on, because stack cleanup cleans *the box*, never the block the
box points at.

**A pointer does not mean "heap".** A pointer is just an address box; it can hold the
address of anything, including a stack variable (verified):

```cpp
int x = 42;
int* p = &x;    // & means "address of" — p points at a STACK box. No heap involved.
*p;             // -> 42
```

Only `new` allocates on the heap. And here's the part you've been living with unaware:
**you used the heap through all of lesson 01.** A `std::vector`'s elements live in a
heap block — that is how it can grow — and `std::string`'s characters do too. (Python
goes all the way: every object you ever made in the Python drills lived in its process's
heap — the "pool" the garbage collector patrols is exactly this region.) You never wrote
`delete` because the containers freed their blocks themselves. How they pull that off is
exactly where this lesson is headed.

**How you give memory back.** Hand the address to `delete` — check out of the room:

```cpp
delete p;       // ledger marks the block free; the allocator can reuse it
```

`new` also has an array form, and it must be paired with the array form of delete:

```cpp
int* a = new int[1000];   // heap-allocate a block of 1000 ints
a[0] = 7;                 // index it like an array
delete[] a;               // array new -> array delete. Mismatching is undefined behavior.
```

That `delete` line is the whole problem. Nothing reminds you to write it. Nothing stops
you writing it twice. The compiler accepts every wrong variant silently. And one detail
deserves emphasis, because everything in the next section turns on it: **`delete` needs
the address as its argument.** The room can only be checked out by presenting the room
number. Lose every copy of the address, and the block is occupied forever — no code can
ever free what it cannot name.

### 3. The two classic disasters

**Disaster 1: the leak.** You allocate and never free. The block stays allocated forever
— no collector exists to notice it's unreachable. This program was compiled and run;
its peak memory was measured at about **800 MB**, for a program whose live data is one
4 MB block:

```cpp
for (int i = 0; i < 250; ++i) {
    int* block = new int[1'000'000];              // 4 MB
    for (int j = 0; j < 1'000'000; ++j) block[j] = j;
    // no delete[] — and `block` is overwritten next turn, so the address is LOST
}
// runs "fine". No error, no warning. The memory is simply gone until the process exits.
```

Walk one lap of the loop in slow motion, watching the two pieces separately — the 8-byte
pointer box on the stack, and the 4 MB block on the heap:

1. Iteration 0: `new` marks a 4 MB block occupied and returns its address — call it
   `A0`. The box `block` now holds `A0`. The inner loop fills the block.
2. The iteration ends. The *box* `block` dies with its scope — but destroying a pointer
   destroys the address *copy*, never the block. The 4 MB at `A0` still sits in the
   ledger as occupied.
3. Iteration 1: a fresh `block` box; `new` hands out a *different* 4 MB block at `A1`.
   And now the trap has closed: **no variable anywhere holds `A0` anymore.** `delete[]`
   needs the address as its argument — a room can only be checked out with its room
   number, and the program just lost the only copy. The `A0` block is unreachable *and*
   unfreeable: leaked.
4. Repeat 250 times: 250 blocks × 4 MB = roughly one gigabyte requested and never
   returned. The ~800 MB peak measured for this program is that pile, as the operating
   system accounts it — while the program's *useful* data at any moment was a single
   4 MB block.

Why does it "run fine"? Because the allocator's ledger only records *occupied or free*.
Nobody audits whether you still hold the address; no collector exists to trace what's
reachable. Run the same loop in Python and nothing leaks: the moment `block` is rebound,
the old object's last reference is gone and Python frees it on the spot (verified). That
per-assignment bookkeeping, running behind every line you write, is exactly the cost C++
declined to pay. No garbage collector, so no surprise pauses — and this is the bill for
it.

One honest question remains: the comment says the memory is gone *"until the process
exits"* — so does the heap get cleaned up at the end? **Yes.** When a process exits, the
operating system reclaims its entire address space — heap, stack, leaks and all. Leaks
never survive the program. They matter for one reason: **the programs that matter don't
exit.** A robot's perception daemon runs for days. Leak this loop's 4 MB per camera
frame at 30 fps and you consume ~120 MB every second — on a 16 GB machine, the process
is dead in about two minutes. A leak never announces itself; the process just grows —
a slow-motion crash.

**Disaster 2: the double free.** Two places both free the same block. The allocator's
bookkeeping is corrupted, and it kills the process. Verified — this program prints its
first two lines, then dies on the second `delete` (exit code 133, no exception, no
recovery):

```cpp
int* p = new int(5);
std::cout << "first delete" << std::endl;
delete p;
std::cout << "second delete" << std::endl;
delete p;                                  // process KILLED here
std::cout << "never printed" << std::endl;
```

(Close cousin: the **dangling pointer** — keeping an address after freeing the block, and
reading through it later. That's undefined behavior: the language makes no promises —
crash, garbage values, or code that works for months and fails in a demo are all allowed.)

Look at what both disasters have in common: the same block had an unclear number of
responsible parties — zero for the leak, two for the double free. So modern C++ enforces
one discipline: **ownership**. Every heap block has *exactly one* owner at any moment.
The owner — and only the owner — frees it. The rest of this lesson is the machinery that
makes the compiler enforce that discipline for you, starting with the hook it all hangs
on.

### 4. The destructor: a hook that runs at the closing brace

A **destructor** is a method named `~ClassName()` that the compiler calls automatically
at the exact moment an object dies. For a local variable, that moment is the closing
brace of its **scope** — the `{ ... }` region where the variable exists. You never call
it; the closing brace does. Verified output, with the construction messages interleaved
(`ctor`/`dtor` are just common shorthand for constructor/destructor, not keywords):

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

Reverse order matters: `c` was built after `b` and might depend on it, so `c` is torn
down first. Later-built objects always die before the objects they might lean on.

And the guarantee holds even when the scope exits *sideways*. If an exception is thrown,
destructors of everything in the scopes being exited still run on the way out — verified:

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

This is the deterministic cleanup the garbage-collected world can't give you as a
default: not "eventually", not "when the collector runs", but *on that brace, in that
order, every time, even mid-exception*. Cleanup rides on scope.

If that exception-proof guarantee feels familiar, it should — Python has one construct
that makes the same promise, and you have typed it a thousand times: `with`. That
parallel is the key to the next section.

### 5. RAII: tie every resource to a scope

**RAII** stands for "Resource Acquisition Is Initialization" — a terrible name for a
simple pattern: *put the resource inside an object*. The constructor acquires the
resource (opens the file, locks the lock, allocates the memory); the destructor releases
it. Section 4's scope rules then guarantee the release with no cleanup code at the call
site, ever — you cannot forget to free something you never had to remember.

You already know this pattern from Python: it is the `with` statement. `with open(path)
as f:` acquires the file in `__enter__` and guarantees its release in `__exit__` — even
when an exception flies out of the block, which is exactly what makes it safe to pair
with the try-first EAFP style from Python drill set 05. RAII is that idea promoted from
a special statement to *every object in the language*: the destructor is an `__exit__`
you never have to remember to ask for. No `with` line to forget, no extra indentation —
**scope itself is the with-block**. Python guarantees cleanup for the objects you
explicitly wrap; C++ guarantees it for everything.

The entire standard library is built on this one mechanism:

| RAII class | Constructor acquires | Destructor releases |
|---|---|---|
| `std::ifstream` | opens the file | closes it |
| `std::lock_guard` | locks a **mutex** (a lock only one thread may hold at a time) | unlocks it |
| `std::vector` | (grows) the heap block for its elements | frees it |
| `std::unique_ptr` | takes charge of a heap object | deletes it |

Read that third row again: you have been using RAII since lesson 01. A `vector`'s
elements live on the heap — that's how it grows — yet you never freed anything, because
the vector's destructor did it at every closing brace. Now we meet the last row: RAII
applied to a single heap object.

### 6. `std::unique_ptr`: a pointer that owns what it points to

`std::unique_ptr<T>` is a pointer that owns what it points to and deletes it in its
destructor. That's the entire concept. It behaves like the raw pointer from section 2 —
holds an address, can be dereferenced, can be null — but when it dies at its closing
brace, it frees the object it points at. Ownership as a type.

You almost never write `new`, and *never* `delete`. The one-step allocator is
`std::make_unique<T>(constructor-args)` — it heap-allocates a `T` and hands you the
owning pointer:

```cpp
auto p = std::make_unique<std::string>("hi"); // heap-allocate a string("hi"); p owns it
std::cout << p->size();       // -> 2      `->` reaches members through a pointer
std::string* raw = p.get();   // borrow the raw address; ownership does NOT change
if (p) { /* runs */ }         // a unique_ptr converts to true when non-null
p.reset();                    // free the string NOW; p becomes null
// (p == nullptr) -> true        nullptr is C++'s typed null literal
```

There is a second form for arrays: `std::unique_ptr<int[]>` (note the `[]`). It calls
`delete[]` — the correct array flavor from section 2, chosen for you — gives you plain
indexing, and `std::make_unique<int[]>(n)` **zero-initializes**: every element starts
at 0 (verified):

```cpp
auto a = std::make_unique<int[]>(5);
// a[0] + a[1] + a[2] + a[3] + a[4] -> 0   (zero-initialized)
a[0] = 7;                                // index it like a plain array
```

The crucial rule: **a `unique_ptr` cannot be copied**. A copy would mean two sole owners
— which is the double-free disaster waiting for two closing braces. So the copy operation
is deleted, and this is a *compile* error, not a runtime surprise:

```cpp
auto p = std::make_unique<int>(1);
auto q = p;   // error: call to implicitly-deleted copy constructor of 'unique_ptr<int>'
```

Stop and admire that: the disaster from section 3 is now a red squiggle. The program
that double-frees does not compile.

And it costs nothing: `sizeof(std::unique_ptr<int>) == sizeof(int*)` — both 8 bytes on
this Mac (verified). Same address a raw pointer holds, plus a destructor. C++ calls this
a *zero-cost abstraction*: the safety is free at runtime, which is why it's acceptable in
a control loop.

(What if two places genuinely must share one object? That exists — `std::shared_ptr`
keeps a count of owners and frees the object when the count hits zero. If that sounds
familiar, it should: a reference count on every object is precisely how Python manages
its whole heap. What Python does for everything, C++ reserves for the rare case of
genuinely shared lifetime — in well-designed code it's the exception, not the default.)

### 7. `std::move`: handing over the keys

If copying is banned, how does a `unique_ptr` ever leave the function that made it? By
**moving**: transferring ownership instead of duplicating it. (Python has no verb for
this — when every assignment shares, nothing is ever *handed over*. Sole ownership makes
the handoff a real operation, so C++ had to give it a spelling.) `std::move(p)` is —
despite the name — just a marker: it moves nothing itself, it only flags `p` as "you may
take from this". The receiving `unique_ptr` does the actual transfer: it takes the
address and nulls out the source. Verified:

```cpp
auto p = std::make_unique<int>(42);
auto q = std::move(p);   // ownership transferred to q
// p == nullptr  -> true    a moved-from unique_ptr is GUARANTEED null
// *q            -> 42
```

Still exactly one owner — it's just a different variable now. And the handoff is visible
in the source: you cannot transfer ownership *silently*, because passing a `unique_ptr`
where a copy would be needed is that same deleted-copy compile error unless you write
`std::move`:

```cpp
void sink(std::unique_ptr<int> owned);   // by-value parameter: this function CONSUMES its argument
sink(p);              // error: call to implicitly-deleted copy constructor
sink(std::move(p));   // OK — and everyone reading this line can SEE the handoff
```

Three gotchas, each one sentence you can act on:

- **After `sink(std::move(p))`, `p` is null.** `*p` still compiles but the process dies
  with a **segmentation fault** — the operating system killing it for touching an invalid
  address. (Verified: exit code 139, no exception, no message.) The compiler made the
  transfer visible; not touching `p` afterwards is on you.
- **Never write `return std::move(local);`** — returning a local `unique_ptr` already
  moves it, and the explicit `std::move` defeats **copy elision** (the optimization where
  the compiler builds the return value directly in the caller's variable, skipping even
  the move). Verified — clang warns: *"moving a local object in a return statement
  prevents copy elision"*. Plain `return p;` is correct.
- **`std::move` doesn't move.** If nothing takes from the marked variable, nothing
  happens at all. It's a label, not an action.

Moving is a much bigger idea than pointers — every container can be moved, and lesson 03
is devoted to it. For now: `std::move` on a `unique_ptr` = ownership transfer.

### 8. The rule of zero

One question remains before the drills: when you write your own resource-owning class,
how much of this machinery must you write by hand? Ideally, none. If a class hand-writes
a destructor, it usually also needs hand-written copy and move rules — in C++ folklore,
the "rule of three/five" (destructor, copy pair, and move pair travel together). The modern escape hatch is the **rule of zero**: make every field a
self-cleaning type (`std::vector`, `std::string`, `std::unique_ptr`) and write *none* of
the five. The compiler-generated destructor destroys each field, each field frees its own
resource, done. The `Buffer` drill below is a rule-of-zero class: it owns heap memory yet
contains no cleanup code whatsoever.

### 9. Syntax you'll need for the drills

- **Member initializer list** — the `: field_(value), ...` between a constructor's
  signature and its body. It *initializes* fields before the body runs (assignment
  inside the body would mean default-construct first, overwrite second). It is the
  required way to set up `const` fields and reference fields, and the idiomatic way for
  everything else:

  ```cpp
  explicit Buffer(std::size_t n) : size_(n), data_(std::make_unique<int[]>(n)) {}
  ```

- **`explicit`** on a one-argument constructor forbids the compiler from silently using
  it as a conversion: without it, `use(5)` would auto-manufacture a `Buffer(5)`; with it,
  that line is an error ("no matching function") and you must write `use(Buffer(5))`.
  Habit: mark every single-argument constructor `explicit`.
- **`const` after a method** — `std::size_t size() const` — promises the method won't
  mutate the object. Calling a non-`const` method on a `const` object or through a
  `const` reference is a compile error ("method is not marked const"), so mark every
  read-only method `const` or callers holding `const Buffer&` can't use it.
- **`= delete`** removes a function at compile time. `ScopedLogger(const ScopedLogger&)
  = delete;` makes any attempted copy the error "call to deleted constructor" — exactly
  how `unique_ptr` bans copying.
- **Reference member** — `std::vector<std::string>& log_;` stores an *alias* to the
  caller's vector: borrowed, not owned, so the destructor has no cleanup duty to it. It
  must be initialized in the member initializer list, and the borrowing object must not
  outlive the thing it borrows.
- **`std::size_t`** is the unsigned (never negative) integer type C++ uses for sizes and
  indexing. **`long long`** is the guaranteed-64-bit integer (lesson 01, step 12).

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
~ClassName() { /* release */ }                // destructor = guaranteed scope-exit hook
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
// Buffer b(5);   b.sum() -> 0   (make_unique<int[]> zero-initializes)
// b.fill(3);     b.sum() -> 15
// b dies at its closing brace -> array freed, no code written for it (rule of zero)
```

Gotcha: `size()` and `sum()` must be marked `const` (they already are in the stub) —
delete the `(void)` placeholder lines once you use the fields for real.

Where you'll see it: the classic C++ interview "implement a String / dynamic array /
Matrix class" is really a resource-ownership test — the follow-up is always "what happens
when I copy it?" (with a `unique_ptr` member: it doesn't compile, which is often the
answer they want, plus how you'd add a deliberate **deep copy** — a hand-written copy
constructor that allocates a fresh array and duplicates the elements, instead of the
banned pointer copy). In robotics/ML work this class *is* the shape of an image frame, a
point-cloud buffer, or a tensor input/output staging buffer — inference APIs hand you raw
memory (often on the GPU) and production code wraps it in exactly this kind of owning
class.

### Drill 2 — `makeBuffer`: a factory that hands ownership out

Task: heap-allocate a `Buffer` and return it as `std::unique_ptr<Buffer>`.

```cpp
std::unique_ptr<Buffer> makeBuffer(std::size_t n) {
    return std::make_unique<Buffer>(n);   // plain return — NO std::move here
}
// auto buf = makeBuffer(4);   buf->size() -> 4
```

Gotcha: writing `return std::move(...)` on a local triggers the "prevents copy elision"
warning from section 7 — a plain `return` of a local `unique_ptr` moves automatically.

Where you'll see it: factory-pattern questions, and the standard screener "why return
`unique_ptr` instead of a raw pointer?" (the signature documents that the caller now owns
it, and the object can't leak even if the caller ignores the result). In real systems,
driver and plugin factories ("give me a camera object for this config") return
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
touching `buf`'s contents after the move is a crash. Both from section 7.

Where you'll see it: the bread-and-butter C++ interview trio — "what does `std::move`
actually do?", "what state is a moved-from object in?", and "implement `unique_ptr`". In
robotics this "sink function" signature is everywhere: ROS 2 (the standard robotics
middleware) lets you `publish()` a `std::unique_ptr` message precisely so the framework
can pass your buffer to subscribers without copying, and pipeline stages (capture →
preprocess → inference) hand frames along the same way.

### Drill 4 — `ScopedLogger`: the RAII scope guard

Task: constructor appends `"enter"` to the borrowed log vector, destructor appends
`"exit"`. Copying is already `= delete`d in the stub. In Python terms you are writing
`__enter__` and `__exit__` — except no caller ever needs a `with` line, because the
brace is the block.

```cpp
std::vector<std::string> log;
{
    ScopedLogger logger(log);   // log: ["enter"]
    log.push_back("work");
}                               // destructor fires AT this brace
// log -> ["enter", "work", "exit"]
```

The nested case in `main()` checks reverse destruction order from section 4: two nested
loggers produce `enter, enter, exit, exit` — the inner one exits first.

Where you'll see it: "What is RAII?" is arguably the single most common C++ interview
question, and "implement `lock_guard`" / "make this cleanup exception-safe" are its
hands-on forms — this drill is that exact exercise. In real code this pattern is
`std::lock_guard` around shared state in every multithreaded control loop, scoped timers
that log how long an inference call took, and safety guards like "stop the motors when
this scope exits, even if an exception is flying".

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

## The road ahead

RAII is the exact pattern you will use on the GPU. CUDA's memory API is section 2 all
over again — `cudaMalloc` hands you a raw address on the device, `cudaFree` gives it
back, and nothing reminds you to call it: the GPU has no garbage collector either, and a
leaked device buffer is gone until the process dies. So real inference code wraps device
memory in exactly what you built here — a `Buffer`-shaped RAII class (or a `unique_ptr`
with a custom deleter that calls `cudaFree`), owning the allocation, freeing it at a
brace you can point to. `ScopedLogger` scales up too: CUDA timing events and profiler
ranges are scope guards in production engines. Master these four drills and you have
already written the memory-management layer of an inference runtime — just with `int`
instead of `float16` and one machine instead of a GPU.

Next lesson: moving is bigger than pointers — move semantics for every type, and what
"stealing the guts" of a vector actually means.
