# C++ Foundations for Inference and Robotics

This is a standalone introduction. It assumes you can program — variables, loops,
functions, classes — but have never had to think about memory. That is the one new
skill C++ demands. It is also the skill that unlocks GPUs.

Every code snippet in this document was compiled and run with `clang++ -std=c++17`
before it was written down. You can trust the behavior shown.

---

## 1. Why C++ — and why inference engineers can't skip it

A robot arm must react in a few milliseconds, every time. An inference server must
answer within its latency budget, every time. These are deadlines.

Most popular languages use a garbage collector. A garbage collector is a background
system that finds unused memory and reclaims it. It runs when *it* decides to. When
it runs, your program can pause. A pause at the wrong moment blows the deadline.

C++ has no garbage collector. Nothing runs behind your back. Every cost in a C++
program is a line you wrote, paid at the moment that line executes. This is called
**deterministic cost**, and it is why the fast layer of the ML world is C++:

- PyTorch's core (ATen, the dispatcher, the CUDA kernels) is C++.
- TensorRT, ONNX Runtime, and llama.cpp are C++.
- vLLM's paged-attention kernels are C++/CUDA.
- ROS, the standard robotics framework, is C++.
- CUDA itself **is** C++ with GPU extensions.

There is a second reason to learn it. C++ makes you deal with memory directly:
where bytes live, when they are copied, when they are freed. That knowledge is not
a hazing ritual. It is literally how computers work — and it is the entry
requirement for CUDA, where managing memory *is* the job.

So the pitch is simple. Learn C++ and you learn the machine. Learn the machine and
GPUs stop being magic.

## 2. The one mental model everything builds on

Here is the model. Everything else in this document is a corollary of it.

> **A variable is a box of bytes at a fixed address.**

Not a label. Not a handle to an object living somewhere else. The variable *is* the
bytes. `int x = 7;` reserves 4 bytes somewhere and writes the value 7 into them.
Ask for the address with `&x` and you get a real number, like `0x16b4deaac`.

**Assignment copies the bytes.** `int y = x;` makes a second, independent box and
copies the 4 bytes into it. Change `y` and `x` does not move. For bigger objects
the copy is bigger, but the rule is the same: assignment duplicates.

Where do the boxes live? Two places.

**The stack** is each function's scratch space. Entering a function claims a slab
of it; local variables are boxes inside that slab. Returning releases the whole
slab instantly — no cleanup pass, no bookkeeping. Stack allocation is nearly free.
Its limit: the size must be known up front, and the space dies with the function.

**The heap** is memory you request at runtime: "give me 4000 bytes." The allocator
finds a free block and hands you its address. The block survives until someone
returns it. That flexibility costs more, and the *return it* part is where all the
famous C++ bugs live. Section 6 covers how modern C++ eliminated them.

```text
 stack (per function, auto-freed)     heap (requested, must be returned)
 ┌─────────────────────────────┐      ┌──────────────────────────────┐
 │ x: 7                        │      │                              │
 │ v: [size=1000, ptr] ────────┼─────▶│ [1000 ints, 4000 bytes]      │
 └─────────────────────────────┘      └──────────────────────────────┘
```

That picture is real. A `std::vector<int>` with 1000 elements is a small box on
the stack — 24 bytes on my machine, verified with `sizeof` — holding the size and
a heap address. The 1000 ints live in the heap block it points to. The vector
*manages* that block: it frees it automatically when the vector's own box dies.

Hold onto this model. Copies, references, pointers, and ownership — the next four
sections — are each just one question about boxes: *same box or new box, and whose
job is it to free the heap ones?*

## 3. Value semantics: copies are real copies

```cpp
std::vector<int> a = {1, 2};
std::vector<int> b = a;   // runs vector's copy code: new heap block, elements copied
b.push_back(3);           // b is {1, 2, 3}
                          // a is still {1, 2} — verified
```

**Mechanism.** `b = a` creates a fully independent vector. For plain types the
compiler copies the bytes. For a vector, assignment runs its copy constructor,
which allocates a fresh heap block and copies every element into it. Two boxes,
two heap blocks, no sharing.

**Why designed this way.** Objects that never share by accident are easy to reason
about — no "I changed it over here and it broke over there." And when a copy is a
line of code you wrote, its cost is visible. C++ refuses to hide costs.

**Consequence.** Passing an argument by value is also a copy. For a 1 GB tensor
that is catastrophic, so C++ gives you references (next section) to opt out.

**In inference.** Accidental tensor copies are a top real-world performance bug.
C++ makes every copy something you typed, so you can find them and delete them.

## 4. References and `const`: another name for the same box

```cpp
int x = 10;
int& r = x;        // r IS x — same box, same address (verified: &r == &x)
r = 42;            // x is now 42

void resetAll(std::vector<int>& v);        // works on the caller's vector
void report(const std::vector<int>& v);    // reads it, cannot change it, no copy
```

**Mechanism.** A reference is a second name for an existing box. It is not a new
box. Under the hood the compiler passes the address, but the language hides that:
you use `r` exactly like `x`. A reference must be bound to a real box when created
and can never be null or re-pointed. `const T&` adds a compile-time promise: reads
only. Break the promise and the code does not compile — verified; `push_back` on a
`const std::vector<int>` is rejected with an error.

**Why designed this way.** You need a way to hand a function your actual object —
either to let it modify it, or just to avoid copying something huge. References do
both, with none of the dangers of raw addresses. `const` exists so the *compiler*
enforces "this function only reads," instead of a comment hoping it is true.

**The default habit.** Function inputs you only read: take `const T&`. Small cheap
things (`int`, `double`): take by value. Mark every method that doesn't mutate its
object as `const`. Interviewers look for exactly this.

**In inference.** Every serious C++ API passes tensors and configs as `const T&`.
A frame of camera data enters the pipeline once and is *referenced* thereafter.

## 5. Pointers: a box holding an address

```cpp
int x = 7;
int* p = &x;   // p is its own box; its VALUE is x's address
*p = 9;        // "go to the address in p, write 9" — x is now 9 (verified)
```

**Mechanism.** A pointer is an ordinary box, 8 bytes on modern machines, whose
content is an address. `&x` produces an address; `*p` follows one. That is the
entire feature. Unlike a reference, a pointer can be null (`nullptr`, pointing at
nothing), can be re-pointed, and supports arithmetic: `p + 1` is the address 4
bytes later, which is how arrays are walked.

**Why do pointers exist when references are safer?** Because some things a
reference cannot express:

- *Maybe-absent*: a pointer can be `nullptr`; a reference always refers.
- *Re-pointable*: a pointer can walk down an array or a linked structure.
- *Heap access*: the heap hands you addresses. Something must hold them.
- *Hardware*: a device, a DMA buffer, a GPU allocation — all just addresses.

**The modern rule.** A raw pointer means *borrowing*: you may look, you never
free. Owning heap memory through raw pointers is the disease; the next two
sections are the cause and the cure.

**In inference.** `tensor.data_ptr()` — the address of the first element — is how
every kernel receives its data. CUDA APIs traffic almost entirely in pointers.

## 6. `new`/`delete`, and why manual memory went wrong

```cpp
int* buf = new int[1000];   // allocator reserves heap block, returns its address
// ... use buf ...
delete[] buf;               // returns the block; forget this and it leaks
```

**Mechanism.** `new` asks the heap allocator for a block, runs constructors, and
gives you the address. `delete` runs destructors and returns the block. The
allocator does not watch you. It hands over the block and forgets you exist.

**What goes wrong.** Three classic failures, all silent:

- **Leak** — you never call `delete`. The block stays reserved forever. A robot
  process leaking a few KB per frame dies hours into a run.
- **Double free** — you `delete` the same block twice. The allocator's records
  are corrupted; the crash comes later, somewhere unrelated.
- **Use-after-free** — you keep using the address after `delete`. Reads return
  garbage; writes scribble over whatever lives there now.

These stay hidden because early returns, exceptions, and refactors make "did every
path free this?" impossible to eyeball. Decades of production crashes proved that
humans cannot do this reliably by hand.

**Why the language has it anyway.** Explicit allocation is the source of C++'s
power — you decide exactly when the expensive operation happens. The fix, next,
keeps the control and removes the human from the release step.

## 7. RAII and smart pointers: the destructor is a hook that always runs

Every C++ class may define a **destructor** — code that runs automatically at the
exact moment an object's box dies: end of scope, in reverse creation order, and
*also* mid-flight when an exception is unwinding the stack. All verified:

```cpp
struct FileLike { ~FileLike() { std::printf("cleanup ran\n"); } };

void risky() {
    FileLike f;
    throw std::runtime_error("boom");
}
// output: "cleanup ran" — THEN the exception reaches the catch block
```

**RAII** (Resource Acquisition Is Initialization) is the idiom built on that hook:
a constructor acquires a resource, the destructor releases it. Wrap any resource —
heap block, file, lock, GPU buffer — in an object, and cleanup becomes impossible
to forget, on every path, exception or not. This is C++'s deepest idea.

The standard library ships RAII wrappers for heap memory, called smart pointers:

```cpp
auto u = std::make_unique<Robot>("arm");  // heap Robot; u's destructor deletes it
auto s = std::make_shared<Robot>("arm");  // ref-counted; freed when LAST owner dies

auto w = u;               // COMPILE ERROR (verified): two sole owners is a lie
auto v = std::move(u);    // ownership transferred; u is now null (verified)
Robot* peek = v.get();    // borrow the raw address; never delete it
```

**Mechanism.** `unique_ptr` is a struct holding one raw pointer, whose destructor
calls `delete`. That is all. It compiles to the same machine code as a careful
hand-written raw pointer — you pay nothing for the safety. Copying it is banned at
compile time because a copy would mean two "sole" owners and a double free.
`shared_ptr` adds a shared counter: each copy increments, each destructor
decrements, the one that hits zero frees.

**Why designed this way.** The compiler already knows exactly where every scope
ends. Placing the cleanup call is a bookkeeping job — so C++ gave the bookkeeping
to the machine that never forgets, and left the *policy* (what is owned, and by
whom) in your hands, in the type system, where reviewers can read it.

**The modern rule.** You essentially never write `new`/`delete`. Containers own
their elements; `unique_ptr` owns single heap objects; `shared_ptr` is for the
rare case of genuinely shared lifetime. Lesson 02 builds an RAII type from
scratch so the magic becomes mechanism.

**In inference.** GPU memory, file handles, CUDA streams, mutexes — production
inference code wraps every one in an RAII type. `torch::Tensor` is at heart a
`shared_ptr` around a device buffer.

## 8. The compile-link model: why headers exist

C++ builds in two separate stages. Knowing them turns build errors from voodoo
into diagnosis. All verified with the exact commands shown:

```cpp
// robot.h — DECLARATIONS: names and shapes, no bodies
#pragma once                    // "include me at most once per file"
int maxTorque(int jointId);

// robot.cpp — DEFINITIONS: the actual bodies
#include "robot.h"
int maxTorque(int jointId) { return jointId * 10; }

// main.cpp — a user
#include "robot.h"
int main() { return maxTorque(3); }
```

```sh
clang++ -std=c++17 -c robot.cpp   # stage 1: compile → robot.o
clang++ -std=c++17 -c main.cpp    #                   → main.o
clang++ robot.o main.o -o app     # stage 2: link the pieces → runnable app
```

**Stage 1 — the compiler** processes one `.cpp` file at a time, in isolation.
`#include` literally pastes the header's text in, so the compiler knows the
*signature* of `maxTorque` and can type-check the call. It emits a `.o` file:
machine code plus a note, "I call a symbol named `maxTorque(int)`."

**Stage 2 — the linker** takes all the `.o` files and matches notes to bodies.
Compile `main.cpp` alone and link it without `robot.o`, and you get — verified:

```text
Undefined symbols for architecture arm64:
  "maxTorque(int)", referenced from: _main in main.o
```

That is the most feared C++ error, and it means one precise thing: *the compiler
saw a declaration, but no `.o` given to the linker contained the definition.*

**Why designed this way.** Independent compilation means a thousand-file project
recompiles only the files you touched, in parallel. Headers are the contract that
makes isolation possible. The lessons in this track are single-file, so you dodge
all of this — but every real codebase is built exactly this way.

**In inference.** `#include <torch/torch.h>` then link `libtorch` — declarations
from the header, definitions from the prebuilt library. Same two stages.

## 9. The workhorse containers

Four types cover most real code. Each is a stack box managing a heap block.

| Container | What it is | Cost intuition |
|---|---|---|
| `std::vector<T>` | growable array, elements contiguous in one heap block | index `O(1)`; `push_back` amortized `O(1)`; contiguous = cache-friendly = **the default container** |
| `std::string` | mutable text, same layout idea as vector | `+=` appends in place; `==` compares contents; `s.substr(i, len)` — second arg is a *length* |
| `std::unordered_map<K,V>` | hash table | average `O(1)` insert/lookup; no ordering |
| `std::map<K,V>` | balanced tree | `O(log n)`; iterates in sorted key order (verified) |

Behaviors worth committing to memory now — each one verified:

```cpp
std::vector<int> v = {1, 2, 3};
v[99];          // unchecked: undefined behavior, may "work", may corrupt
v.at(99);       // checked: throws std::out_of_range — use while learning

std::unordered_map<std::string, int> counts;
++counts["hits"];        // operator[] INSERTS a zero if absent, then increments —
                         // perfect for counters, a surprise in lookups
counts.at("missing");    // throws if absent; counts.find(k) returns end() if absent

for (const auto& [key, value] : counts) { ... }   // structured bindings: unpack pairs
```

One trap with a name: **iterator invalidation**. Growing a vector may move its
heap block to a bigger home. Every pointer, reference, or iterator into the old
block now dangles. Rule: do not hold positions into a vector across `push_back`.

Also in the toolbox when you need them: `std::set`/`std::unordered_set` (keys
only), `std::deque` (fast at both ends), `std::priority_queue` (a heap —
**max**-first by default), `std::array<T, N>` (fixed size, lives on the stack),
`std::optional<T>` (a maybe-value without the heap). Algorithms are free
functions: `std::sort(v.begin(), v.end())`, `std::find`, `std::max_element`.

**In inference.** A tensor is conceptually a `vector<float>` plus shape metadata.
Contiguous memory is *the* reason: GPUs and CPU caches both devour sequential
bytes. Lesson 05 measures exactly how much that matters.

## 10. Templates: compile-time code generation

```cpp
template <typename T>
T maxOf(const T& a, const T& b) { return b < a ? a : b; }

maxOf(3, 5);                    // compiler generates maxOf<int> — real ints
maxOf(std::string("ant"),
      std::string("bee"));      // and a SEPARATE maxOf<std::string>  (verified)
```

**Mechanism.** A template is a recipe, not code. At each use, the compiler stamps
out a fresh copy specialized for the concrete type, then optimizes that copy as if
you had written it by hand. `std::vector<int>` and `std::vector<float>` are two
different generated classes, each storing raw elements contiguously — no wrappers,
no indirection.

**Why designed this way.** One implementation, zero runtime cost per type. The
price: templates must live in headers (the compiler needs the recipe visible at
the point of use), and a type mismatch produces errors deep inside the template —
the origin of C++'s legendarily long error messages. Read the *first* error.

**In inference.** One kernel template serves `float`, `half`, and `int8` at full
speed — this is how every quantization-aware library avoids writing each kernel
three times. Eigen and CUTLASS are templates all the way down.

## 11. Move semantics in three sentences

Copying a million-element vector duplicates a million elements, but *moving* it
just hands the heap-block address to the new owner and leaves the source empty —
verified: after `std::vector<int> b = std::move(a);`, `b.data()` is the exact
address `a.data()` used to be, and `a` is empty. `std::move(x)` moves nothing by
itself; it is a cast that marks `x` as safe to pillage, which is also the only way
to transfer a `unique_ptr`, since copying one is banned. Two rules until lesson
03 makes this rigorous: after moving from a variable, only assign to it or let it
die; and never write `return std::move(local);` — plain `return local;` already
moves or better.

## 12. The zero-overhead principle: the philosophy under all of it

One sentence explains every design choice above:

> **You don't pay for what you don't use, and what you do use could not be
> hand-coded faster.**

Watch it operate. References compile to plain addresses. `unique_ptr` compiles to
a raw pointer whose `delete` the compiler places for you. Templates compile to
exactly the specialized code you would have written per type. Destructor calls
are inserted at compile time, not discovered by a runtime. `const` vanishes
entirely after compilation — it exists only to catch your mistakes early.

Safety in C++ is bought with compile-time proof, not runtime supervision. That is
the whole trade: more thinking up front, deterministic microseconds forever after.
For deadline code — a control loop, a token-generation step — it is the only
trade that works.

## 13. The road to CUDA

Here is the payoff. CUDA is not a new language. **CUDA is C++** plus a way to
launch a function across thousands of GPU cores at once.

Everything in this document transfers directly:

- `cudaMalloc` / `cudaFree` are `new` / `delete` for GPU memory — same contract,
  same leak-and-double-free failure modes, same RAII cure. Production code wraps
  device buffers in `unique_ptr`-style guards exactly as in section 7.
- `cudaMemcpy` is a byte copy between CPU and GPU boxes — and it crosses the PCIe
  bus, so the "find and delete needless copies" instinct from section 3 becomes
  worth *milliseconds* there, not microseconds.
- A kernel receives raw device pointers and computes addresses with the pointer
  arithmetic from section 5.
- Memory layout decides everything: GPUs reach peak speed when adjacent threads
  read adjacent addresses — the contiguous-`vector` intuition, multiplied by ten
  thousand cores.

Learn the material in this track and CUDA becomes a dialect, not a mountain.

## 14. How to use this track

Six lessons, each a directory beside this file. Do them **in order** — each
builds on the previous:

1. `01_stl_containers` — vectors, strings, maps in muscle memory
2. `02_ownership_and_raii` — build an RAII resource type from scratch
3. `03_move_semantics_rule_of_five` — moves, made rigorous
4. `04_virtual_functions_and_vtables` — what polymorphism costs and how it works
5. `05_memory_layout_and_cache` — measure why contiguous memory wins
6. `06_threads_atomics_queues` — concurrency for real-time pipelines

Per lesson: read its `README.md`, then implement `starter.cpp` yourself and run
the tests against *your* code:

```sh
PRACTICE=1 uv run pytest ramp_up/cpp/01_stl_containers -v
```

Without `PRACTICE=1`, pytest checks the reference `solution.cpp` instead — useful
for seeing the target behavior before you start, and for study afterward.

While learning, compile with warnings and sanitizers on:
`-Wall -Wextra -fsanitize=address,undefined`. The sanitizers turn silent memory
mistakes into loud, pinpointed reports — a safety net while the instincts form.

Start with lesson 01. The machine is simpler than it looks from the outside.
