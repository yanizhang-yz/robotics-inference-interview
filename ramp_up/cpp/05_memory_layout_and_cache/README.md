# 05 — Memory Layout and Cache-Friendly Code

This is the heart of "why is C++ fast, and how do I keep it fast." After this lesson
you will be able to: say exactly where any C++ variable lives (stack or heap) and what
that costs, predict `sizeof` for a struct — including the invisible padding — and
shrink it by reordering fields, explain what a cache line is and why touching one byte
drags in 64-128 of them, make a loop several times faster *without changing its Big-O*
by walking memory in the right order, choose between array-of-structs and
struct-of-arrays like an inference engineer, and stop `std::vector` from reallocating
in the middle of a hot loop. Interviewers in this space say it outright: **"cache
locality often matters more than Big-O."** By the end you will have the numbers to
defend that sentence, measured on your own machine.

Every number quoted below was measured on one machine (an Apple M4 Pro laptop,
`clang++ -std=c++17 -O2`). Your constants will differ; the *shapes* — 5x, 20x, 400x —
are what transfer.

## The Java you know

```java
// 1. Every object lives on the heap; every variable holding one is a reference.
Point p = new Point(3, 4);     // heap allocation + a ~12-16 byte object header
Point q = p;                   // copies the REFERENCE — the object never moves

// 2. Even "value" collections are arrays of pointers to boxed objects:
List<Integer> xs = new ArrayList<>();
xs.add(42);                    // an Integer[] of references -> boxed Integers,
                               // each its own heap object, scattered anywhere

// 3. A 2D array is an array of POINTERS to row arrays — separate heap objects:
int[][] grid = new int[1080][1920];

// 4. Field order inside a class is the JVM's business (it silently reorders
//    fields to pack them), there is no sizeof, and the GC relocates objects
//    whenever it likes. Memory layout is simply not in your vocabulary.
```

None of this made Java *wrong* — the JIT and GC do a genuinely good job behind the
blindfold. But you could never *choose* your layout, and layout is where the
performance lives. C++ removes the blindfold: structs are exactly the bytes you
declare, locals really live on the stack, containers put values — not references —
side by side. That control is precisely why robotics and inference shops write C++
for anything inside the frame loop: at 30-60 frames per second, cache-friendly vs
cache-hostile is the difference between hitting the control deadline and missing it.

## The lesson

### 1. Two places a value can live: the stack and the heap

Both languages have the same two memory regions; they just fill them differently.

- The **stack** is the scratch space a function gets when it is called and gives back
  the instant it returns. Allocation is one register bump, the same few kilobytes get
  reused by every call so they are almost always already in cache, and cleanup is
  free — the function returns, the space is gone.
- The **heap** is the big shared pool for data that must outlive a function call or is
  too large for the stack (the stack is small — around 8 MB per thread by default).
  Heap allocation means asking an allocator to find a free block — much more work
  than a register bump.

In Java, only *primitive* locals (`int`, `double`, references themselves) lived on
the stack. Every object — every `new Point(...)`, every array, even a boxed
`Integer` — went to the heap, behind a reference. In C++, **local variables live on
the stack no matter their type**, whole structs and arrays included:

```cpp
struct Pose { float x, y, theta; };      // sizeof(Pose) == 12 — three floats,
                                         // no header, nothing else

void control_step() {
    float speed = 0.5f;                  // stack
    Pose target{1.0f, 2.0f, 0.1f};       // stack: 12 bytes, ZERO allocations
    Pose waypoints[8];                   // stack: 96 bytes, still zero allocations

    std::vector<float> big(1'000'000);   // lives in BOTH places at once:
    // `big` itself: 24 bytes on the stack (three pointers: begin, end, cap-end)
    // its DATA: 4 MB on the heap, in one contiguous block
}   // stack variables vanish for free; the vector's destructor releases the
    // 4 MB immediately (that's RAII — lesson 02)
```

The deeper shift: in Java, an object field of object type is always a *reference* to
somewhere else. In C++, a struct member is **embedded in place** — a
`struct Robot { Pose pose; ... }` contains the 12 bytes of `pose` inside itself, and a
`std::vector<Pose>` is one solid slab of 12-byte records shoulder to shoulder. Java's
`ArrayList<Pose>` would be an array of pointers to 1000 separate objects. Hold onto
that picture; sections 3-6 are all consequences of it.

### 2. `sizeof`, alignment, and the padding the compiler sneaks in

**`sizeof(T)`** is a compile-time operator giving the size of a type in bytes. Java
has no equivalent (you need a library like JOL to even inspect layout). On every
mainstream 64-bit platform: `char` is 1, `int` and `float` are 4, `double`, `long
long`, and every pointer are 8.

So a struct holding `char + double + char + int` should be 1+8+1+4 = 14 bytes? No:

```cpp
struct BadOrder {      // total: 24 bytes (!)
    char   ready;      // offset 0
    double timestamp;  // offset 8   <- 7 wasted bytes before it
    char   flags;      // offset 16
    int    id;         // offset 20
};
```

Two definitions explain the missing 10 bytes:

- **Alignment**: each type has a rule "my address must be divisible by N" — for
  `double`, N is 8; for `int`, 4; for `char`, 1 (`alignof(double) == 8`). Hardware
  loads are fastest (on some chips, only legal) at aligned addresses.
- **Padding**: unused filler bytes the compiler inserts so every field lands on its
  alignment. The struct's total size also rounds up to a multiple of its largest
  field alignment, so that in an array `element[1]` starts aligned too.

And one C++ rule that surprises Java developers: **the compiler must keep your
declaration order** (a C-compatibility guarantee). The JVM reorders fields for you;
C++ hands you the packing job. Sort fields largest-alignment-first and the padding
melts away:

```cpp
struct GoodOrder {     // total: 16 bytes — same four fields, 33% smaller
    double timestamp;  // offset 0
    int    id;         // offset 8
    char   ready;      // offset 12
    char   flags;      // offset 13   <- 2 tail-pad bytes round 14 up to 16
};
// Verified with clang++ -std=c++17 -O2:
//   sizeof(BadOrder) == 24, sizeof(GoodOrder) == 16
// Smaller demo of the same rule:
//   struct {char a; int b; char c;}  -> 12 bytes
//   struct {int b; char a; char c;}  ->  8 bytes
```

Why care about 8 bytes? Multiply by a million detections per minute of video, then
read section 3: smaller structs mean more elements per cache line, and cache lines
are the currency everything below is priced in.

### 3. The cache line: memory arrives in chunks, never bytes

Three definitions, then the single most important measurement in this lesson.

- The **CPU cache** is a small, fast memory on the CPU chip that hardware
  automatically fills with copies of recently used main-memory data. There are
  levels (L1, L2, ...) from tiny-and-1-nanosecond to bigger-and-slower; main memory
  (DRAM) is on the order of 100 ns away — a factor of about a hundred.
- A **cache line** is the unit caches actually move: a fixed-size block — 64 bytes
  on most x86 machines, 128 on Apple Silicon (this Mac reports it directly:
  `sysctl -n hw.cachelinesize` → `128`). Touch *one byte* and the CPU loads the
  *entire line*. A **cache hit** means the line was already on-chip (fast); a
  **cache miss** means a trip to DRAM (slow).
- **Spatial locality** is the payoff of the line: after you touch address A, the
  neighbors of A are already loaded — reading them is free. And the **prefetcher**
  — hardware that watches your access pattern — recognizes sequential walks and
  fetches the *next* lines before you ask, hiding DRAM latency entirely.

Now the measurement. The program below visits 2 million slots in a 256 MB table.
Each slot stores the index of the next slot to visit — a linked list without the
syntax, because each load's *address* depends on the previous load's *result*. The
only difference between the two runs is the *order* of the hops:

```cpp
#include <algorithm>
#include <chrono>
#include <cstdio>
#include <numeric>
#include <random>
#include <vector>

int main() {
    const std::size_t N = 32'000'000;              // 32M slots x 8 bytes = 256 MB
    std::vector<std::size_t> seq(N), rnd(N), order(N);
    for (std::size_t i = 0; i < N; ++i) seq[i] = (i + 1) % N;   // ring, in order
    std::iota(order.begin(), order.end(), std::size_t{0});      // ring, shuffled
    std::shuffle(order.begin(), order.end(), std::mt19937_64(42));
    for (std::size_t k = 0; k < N; ++k) rnd[order[k]] = order[(k + 1) % N];

    for (auto* table : {&seq, &rnd}) {
        auto t0 = std::chrono::steady_clock::now();
        std::size_t i = 0;
        for (int s = 0; s < 2'000'000; ++s) i = (*table)[i];    // dependent hops
        std::printf("%.1f ms (%zu)\n",
                    std::chrono::duration<double, std::milli>(
                        std::chrono::steady_clock::now() - t0).count(), i);
    }
}
// On one machine (clang++ -O2): sequential 0.5 ms, shuffled 232.9 ms — ~460x.
```

Same 2 million loads, same instructions, same Big-O — **460x apart**. Sequential
hops ride the cache line (16 slots per 128-byte line) and the prefetcher; shuffled
hops pay a full ~116 ns DRAM round trip *each*, one at a time, because the CPU
cannot even start fetching hop N+1 until hop N delivers the address. That
per-load-at-a-time serialization is called **pointer-chasing**, and it is the
worst thing you can do to modern hardware.

This hardware ran your Java programs too — you just had no steering wheel: the GC
placed and moved objects, and every object reference was a potential chase. C++
gives you the wheel. The rest of the lesson is learning to steer.

### 4. Consequence 1 — contiguous beats pointer-chasing: `vector` vs `list`

**Contiguous** means "in one unbroken block of memory, element after element."
`std::vector<int>` is contiguous: the ints themselves sit back to back, 32 of them
per 128-byte line. `std::list<int>` is a **doubly-linked list**: every element is a
separate heap allocation holding `{prev pointer, next pointer, value}` — roughly 24
bytes to store 4, and advancing to the next node is exactly the dependent load from
section 3.

Java calibration: this is *not* ArrayList vs LinkedList. `ArrayList<Integer>` also
pointer-chases — it is a contiguous array *of references* to boxed `Integer` objects
scattered across the heap. `std::vector<int>` — actual values, actually adjacent —
is the thing Java never gave you (it is what Project Valhalla has been trying to add
for a decade).

```cpp
std::vector<int> vec(10'000'000);            // fill with i % 256
std::list<int>   lst(vec.begin(), vec.end());

long long sv = 0; for (int v : vec) sv += v;   // 0.4 ms   on one machine, -O2
long long sl = 0; for (int v : lst) sl += v;   // 8.0 ms   same values: ~19x
```

Identical loop bodies, identical sums, ~19x — **and this is the friendly case**: the
list was built in one go, so its nodes came out of the allocator nearly adjacent. In
a long-running program that inserts and erases, the nodes scatter and the walk drifts
toward the 460x chase above.

This is where the interview quote earns its keep. The textbook says a linked list
inserts in the middle in O(1) — but you must *walk* to the middle first (a pointer
chase), while `vector`'s "expensive" O(n) shuffle-down is one streaming pass over
contiguous memory, the pattern hardware loves most. For anything that fits in memory,
`std::vector` wins until proven otherwise. **Default to `vector`; demand evidence
before using anything node-based.**

### 5. Consequence 2 — traversal order: walk the matrix the way it is stored

A 2D image or matrix in C++ is normally one flat contiguous block in **row-major**
order: all of row 0, then all of row 1, and so on, with element `(r, c)` at index
`r * cols + c`. (The transpose convention — column-major — exists too, in Fortran,
MATLAB, and some BLAS libraries.) Java's `float[][]` is neither: it is an array of
pointers to independently allocated row objects — one more chase per row, and rows
may be anywhere. In C++ you use the flat block, which is also how camera frames and
tensors arrive from every real API.

Two loops, same work, opposite orders:

```cpp
// 4096 x 4096 floats (64 MB), summed both ways (clang++ -O2, one machine):
double rows = 0;                              // ROW-MAJOR ORDER: r outer, c inner
for (std::size_t r = 0; r < R; ++r)
    for (std::size_t c = 0; c < C; ++c)
        rows += m[r * C + c];                 // 8.6 ms — addresses increase by 4

double cols = 0;                              // AGAINST THE GRAIN: c outer, r inner
for (std::size_t c = 0; c < C; ++c)
    for (std::size_t r = 0; r < R; ++r)
        cols += m[r * C + c];                 // 50.0 ms — same sum, ~6x slower
```

The first loop touches addresses 0, 4, 8, ... — every byte of every cache line gets
used, and the prefetcher streams lines ahead of the loop. The second hops 16 KB per
step (one full row), using 4 bytes of each line it drags in; by the time it wraps
around to that line's neighbors, the line has long been evicted, so the 64 MB gets
re-fetched over and over.

The rule costs nothing to apply: **make the last index the innermost loop**. For
images (row-major, and taller than a cache line is wide) that means `y` outer, `x`
inner — burn `for (y) for (x) pixel[y * width + x]` into your fingers. The same
decision at framework scale is tensor layout: NCHW vs NHWC is "which dimension sits
contiguous in memory," chosen so the hot kernel walks stride-1.

### 6. Consequence 3 — AoS vs SoA: which fields travel together?

Say your detector emits a struct per object (six 4-byte fields — alignment 4, so
`sizeof(Detection) == 24` with zero padding; predict it yourself using section 2):

```cpp
struct Detection { float x, y, w, h; float score; int class_id; };
```

- **AoS — array of structs**: `std::vector<Detection>` — whole records side by side:
  `x y w h score id | x y w h score id | ...`. Natural to write; each element's
  fields are together.
- **SoA — struct of arrays**: one packed array per field: `std::vector<float>
  scores; std::vector<float> xs; ...` — all scores side by side:
  `score score score ...`.

Now scan 20 million detections for the best score. AoS reads 4 useful bytes per 24 —
it drags 480 MB through the cache to use 80 MB. SoA reads exactly the 80 MB it needs.
Measured honestly, in two acts (one machine, 20M detections):

```cpp
float best = -1.0f;
for (const Detection& d : aos) best = d.score > best ? d.score : best; // 5.4 ms
for (float s        : scores)  best = s      > best ? s      : best;   // 4.6 ms
// Plain -O2: barely different! Each max depends on the previous one — a
// DEPENDENCY CHAIN. The CPU can't start iteration N+1's max until N's is
// done, so the *core* is the bottleneck and the memory difference hides.

// Compile the same code with -O2 -ffast-math (lets the compiler reorder
// float arithmetic, which unlocks SIMD for reductions like max and sum):
//   AoS: 4.9 ms          SoA: 0.85 ms          — ~6x, on one machine
```

**SIMD** (single instruction, multiple data — "vector instructions") lets one
instruction process 4-16 values at once, but only when they sit contiguously. The
packed `scores` array vectorizes; the stride-24 walk through `Detection`s cannot.
That is the full lesson of SoA: **layout doesn't just set your cache traffic — it
decides whether SIMD is possible at all.** Inference runtimes are SoA from top to
bottom; a tensor is SoA taken to its logical extreme (and Java's `ArrayList
<Detection>` — references to scattered objects — is "none of the above": every
element is a pointer chase before you even reach a field).

AoS is still right when the hot loop uses *all* fields of one element together (box
math in NMS, integrating one robot state). The design question to ask out loud in an
interview: **"which fields travel together through the hot loop?"** Fields that
travel together, stay together; a field scanned alone earns its own array.

### 7. The practical rules (and what `reserve()` buys you)

A `std::vector` owns one heap block. `size()` is how many elements you have;
**capacity** is how many fit in the current block. `push_back` onto a full vector
triggers a **reallocation**: allocate a bigger block, copy every element over, free
the old block — and every pointer and iterator into the old block is now invalid
(lesson 02's dangling pointers, again). Growth is geometric — libc++ doubles:

```cpp
std::vector<int> v;                  // capacity 0 — a fresh vector owns no memory
for (int i = 0; i < 1'000'000; ++i) v.push_back(i);
// capacity walks 1, 2, 4, 8, ..., 1048576: 21 reallocations (measured),
// recopying everything each time — ~2 million element copies total.

std::vector<int> w;
w.reserve(1'000'000);                // ONE allocation, up front
for (int i = 0; i < 1'000'000; ++i) w.push_back(i);
// capacity changes exactly once: 0 -> 1000000 at the reserve (measured).
// 10M push_backs, one machine: 13.2 ms without reserve, 6.5 ms with (~2x) —
// and, often more important in robotics, no mid-loop allocation spikes and
// no surprise pointer invalidation.
```

Java parallel: `ArrayList` grows ~1.5x per overflow, and `ensureCapacity(n)` is
exactly `reserve(n)` — you probably never called it because the GC hid the cost.
At a 30-60 Hz control loop, you can't afford hidden costs.

The rules this lesson leaves you with:

1. **Default to `std::vector`.** Contiguity is the whole ballgame (sections 3-4).
2. **`reserve()` when you know the size** — before any big `push_back` loop.
3. **Keep hot structs small** and order fields largest-alignment-first (section 2).
4. **Iterate in storage order** — last index innermost; `y` outer for images.
5. **Split hot fields from cold (SoA)** when a loop scans one field across many
   elements (section 6).
6. **Measure before believing** — including this README: rerun these snippets on
   your own machine. Magnitudes shift; the shapes stay.

## Muscle memory

Type these until they require no thought:

```cpp
sizeof(T); alignof(T);                          // bytes and alignment rule, compile time
struct Hot { double d; int i; char c; };        // largest first — padding melts away
std::vector<int> v; v.reserve(n);               // one allocation, THEN the push_back loop
for (r ...) for (c ...) m[r * cols + c];        // row-major: LAST index innermost
for (y ...) for (x ...) img[y * width + x];     // images: y outer, x inner — always
std::vector<Detection> dets;                    // AoS: all fields travel together
std::vector<float> scores;                      // SoA: one field scanned alone
```

## The drills

Work through `starter.cpp` top to bottom. Timings print so you can *see* the effects,
but every assert checks correctness only — a slow machine never fails the tests.
Compile with `-O2` when you want the timing lines to mean something.

### `padded_size_report()`

Fix `GoodOrder`'s field order (same four fields as `BadOrder`, reordered
largest-first), then return `{sizeof(BadOrder), sizeof(GoodOrder)}`. Predict both
numbers on paper before the compiler grades you.

```cpp
padded_size_report()   // -> {24, 16} once GoodOrder is properly ordered
```

Where you'll see it: "why is `sizeof` of this struct 24 and not 14 — and make it
smaller" is a C++ interview staple, as is reading a struct and computing its size on
a whiteboard. For real: message structs on the wire to a robot arm (CAN/serial
protocols are packed byte layouts), shared-memory frames between a vision process
and a controller, and shrinking a per-detection struct so more of them fit per cache
line in the post-processing loop.

### `sum_rows_first(m, rows, cols)` / `sum_cols_first(m, rows, cols)`

Sum a flat row-major matrix with `r` outer / `c` inner, then with the loops swapped.
Same indexing expression `m[r * cols + c]` in both; both must return the identical
sum (main() checks a small exact case and the big matrix, and prints both timings).

```cpp
sum_rows_first(m, 3, 4)   // == sum_cols_first(m, 3, 4) — order changes time, never truth
```

Where you'll see it: the classic "these two nested loops do the same thing — why is
one 6x slower?" interview question, asked with exactly this transposed-loop code. For
real: every pixel loop you will ever write (images are row-major: `y` outer, `x`
inner), convolution and pooling kernels, and the NCHW-vs-NHWC tensor layout decision,
which is this drill wearing a framework badge.

### `sum_vector(values)` / `sum_list(values)`

Identical accumulation loops over `std::vector<int>` and `std::list<int>`; main()
asserts the sums match and prints both timings — contiguous vs pointer-chase, live.

```cpp
sum_vector({1, 2, 3})   // -> 6
sum_list({1, 2, 3})     // -> 6, eventually — each ++it is a dependent load
```

Where you'll see it: "when would you use `std::list`?" (expected answer: almost
never — and *why*), "ArrayList vs LinkedList" upgraded to systems level, and the
Big-O-vs-locality discussion this whole lesson is named after. For real: point
clouds, detection buffers, joint trajectories — every per-frame collection in a
robotics pipeline is a contiguous buffer, and node-based structures inside a control
loop are a code-review rejection.

### `top_score_aos(dets)` / `top_score_soa(scores)`

Max confidence over a frame, twice: over `vector<Detection>` (AoS) and over the
packed `vector<float>` of scores (SoA). Both return `-1.0f` for an empty frame;
main() asserts they agree exactly and prints both timings.

```cpp
top_score_aos(dets)     // reads 4 useful bytes per 24 — box fields are dead weight
top_score_soa(scores)   // packed floats: 1/6 the traffic, SIMD-ready
```

Where you'll see it: "how would you lay out a million detections?" is a real
inference-engineer interview question, and AoS-vs-SoA by name is a favorite at
robotics and game-engine shops (it is the founding idea of ECS architectures). For
real: NMS and top-k score scans in every detector's post-processing, and the reason
tensors exist at all — a tensor is SoA taken to the limit.

### `fill_with_reserve(n)` / `fill_without_reserve(n)`

Build a vector of `n` ints with `push_back`, counting how many times `capacity()`
changes along the way — with `reserve(n)` first (capacity jumps once: 0 → n,
verified on this machine's libc++) and without (doubling: 1, 2, 4, ... — 21 changes
for a million elements). main() asserts the counts (== 1 with, > 1 without) and
prints both timings.

```cpp
fill_with_reserve(100'000)      // -> 1   one up-front allocation
fill_without_reserve(100'000)   // -> 18  on this machine's libc++ (doubling from 1)
```

Where you'll see it: "you know the final size — reserve it" is among the most common
C++ code-review comments, and "what does push_back cost, amortized, and what happens
to iterators on reallocation?" is a standard screener follow-up. For real: per-frame
detection and point-cloud vectors in a 30-60 Hz loop, where a mid-frame reallocation
is a latency spike and a use-after-free waiting to happen.

## How to practice

```bash
# Against the reference solutions (should pass out of the box):
uv run pytest ramp_up/cpp/05_memory_layout_and_cache -v

# Against YOUR implementation in starter.cpp:
PRACTICE=1 uv run pytest ramp_up/cpp/05_memory_layout_and_cache -v

# Compile and run directly — asserts + timing lines, ALL TESTS PASSED at the end:
clang++ -std=c++17 -Wall -o /tmp/mem starter.cpp && /tmp/mem

# The tests compile without optimization (correctness only). For timing numbers
# worth reading — like the ones quoted above — turn the optimizer on:
clang++ -std=c++17 -O2 -Wall -o /tmp/mem solution.cpp && /tmp/mem
```
