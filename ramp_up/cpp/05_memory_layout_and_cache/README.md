# 05 — Memory Layout and Cache-Friendly Code

This is the heart of "why is C++ fast, and how do I keep it fast." After this lesson
you will be able to: explain what a cache line is and why touching one byte drags in
64-128 of them, make a loop several times faster *without changing its Big-O* by
walking memory in the right order, predict `sizeof` for a struct — including the
invisible padding — and shrink it by reordering fields, choose between
array-of-structs and struct-of-arrays like an inference engineer, and stop
`std::vector` from reallocating in the middle of a hot loop. Interviewers in this
space say it outright: **"cache locality often matters more than Big-O."** By the end
you will have the numbers to defend that sentence, measured on your own machine.

You also arrive with evidence of your own. In the NumPy lesson
([`ramp_up/python/08`](../../python/08_numpy_essentials/)) you measured a pure-Python
loop running well over 100x slower than the NumPy one-liner, and the fix was always
"vectorize". This lesson is where that advice stops being a magic word: to vectorize
is to *hand the loop to compiled code that walks one contiguous block of memory*.
What makes the contiguous block special is exactly what the next sections build.

Every number quoted below was measured on one machine (an Apple M4 Pro laptop,
`clang++ -std=c++20 -Wall -Wextra -Werror=return-type -O2`). Your constants will
differ; the *shapes* — 6x, 19x,
400x — are what transfer.

## Two loops, one mystery

Here are two loops. They add up the same 4096×4096 grid of floats (64 MB). Same
elements, same number of additions, same Big-O, same answer:

```cpp
double a = 0;
for (std::size_t r = 0; r < R; ++r)
    for (std::size_t c = 0; c < C; ++c)
        a += m[r * C + c];               //  8.6 ms on one machine

double b = 0;
for (std::size_t c = 0; c < C; ++c)
    for (std::size_t r = 0; r < R; ++r)
        b += m[r * C + c];               // 50.0 ms — same sum, ~6x slower
```

The only difference is which loop is on the outside. Nothing in the source code
explains a 6x gap — no extra work, no different math. The explanation lives one
level down, in how the hardware moves bytes. This lesson builds that explanation
from scratch, then cashes it in five times. By section 6 the mystery is solved;
by the end, you will be the person who *causes* the fast version on purpose.

## The lesson

### 1. RAM is far away

Start with one uncomfortable fact. Your CPU can do an addition in well under a
nanosecond. But the numbers it adds live in main memory (**DRAM** — the "16 GB of
RAM" on the spec sheet), and fetching one value from DRAM takes on the order of
**100 nanoseconds** — call it 100+ cycles, and at laptop clock speeds it is
realistically several hundred. You will measure almost exactly this on your own
machine in section 4: ~116 ns per fetch.

Sit with the ratio for a second. If the CPU had to wait for DRAM on every value, it
would spend over 99% of its time doing nothing — a chef who can chop in one second
but waits five minutes for each ingredient to be walked in from a warehouse.

*Why is memory designed this way?* Physics and economics. Storage that is large and
cheap (DRAM cells) is slow to reach; storage that is fast sits on the CPU chip
itself and is small and expensive. You cannot have gigabytes at sub-nanosecond
distance. Every computer you will ever program is built around this trade.

*Where this bites in our field:* inference and robotics code is mostly "walk over
big arrays of numbers" — image pixels, tensor weights, point clouds. That work is
usually **memory-bound**: the arithmetic is easy and the bytes are the bottleneck.
Which is exactly why layout, the subject of this lesson, is the lever that matters.

### 2. The cache: a small, fast stash next to the chef

The hardware fix for the warehouse problem is the **CPU cache**: a small, fast
memory *on the CPU chip* that hardware automatically fills with copies of
recently-used main-memory data. When the CPU needs a value, it checks the cache
first. Finding it there is a **cache hit** — roughly a nanosecond. Not finding it
is a **cache miss** — the full trip to DRAM.

There are levels (L1, L2, ...), from tiny-and-instant to bigger-and-slower, but one
mental model carries you through interviews: **a small kitchen counter in front of
a huge slow warehouse**. Anything already on the counter is nearly free. Anything
else costs the walk.

You do not manage the cache — no C++ syntax touches it. Hardware decides what to
keep. But hardware follows simple, predictable rules, and the entire craft of this
lesson is writing code those rules reward. The next two sections are those rules.

### 3. The cache line: memory moves in chunks, never bytes

Rule one: **memory never moves one byte at a time.** It moves in fixed-size chunks
called **cache lines** — 64 bytes on most x86 machines, 128 on Apple Silicon. Ask
your Mac directly; this is a runnable claim:

```bash
sysctl -n hw.cachelinesize     # -> 128 on this machine
```

Touch *one byte*, and the CPU pulls that byte's **entire line** into the cache.
Read `x[0]` in an array of 4-byte ints and you have silently also paid for — and
received — `x[1]` through `x[31]`. They are on the counter now. Reading them next
is nearly free.

*Why chunks?* Because the trip to DRAM is the expensive part, not the bytes
carried. If you are walking five minutes to a warehouse, you bring back a box, not
a single egg. The hardware is betting you will want the neighbors. That bet has a
name — **spatial locality**: programs that touch address A very often touch A's
neighbors next. Code that makes the bet pay wins; code that makes it lose pays
full price per byte.

*Where you'll meet it:* this exact chunk idea reappears at every scale of the
stack — disk pages, network packets, and (the road-ahead section) GPU memory
transactions. Learn it once here, recognize it everywhere.

### 4. Streams beat hops: the prefetcher, and the 400x measurement

Rule two: the CPU watches your access pattern. When you read addresses in
sequence — line, next line, next line — a piece of hardware called the
**prefetcher** notices and starts fetching lines *before you ask for them*. A
sequential walk hides the DRAM distance completely: by the time the loop wants a
line, it is already on the counter.

Now the flip side. What is the *worst* pattern? One where each load's **address
depends on the previous load's result** — because then nothing can be fetched
ahead: the CPU cannot even begin fetch N+1 until fetch N delivers. This pattern is
called **pointer-chasing**, and here it is, isolated and measured. The program
visits 2 million slots in a 256 MB table; each slot stores the index of the next
slot to visit. The only difference between the two runs is the *order* of the hops:

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

Same 2 million loads, same instructions, same Big-O — **hundreds of times apart**.
The sequential run rides the cache line (16 slots per 128-byte line) and the
prefetcher. The shuffled run pays a full DRAM round trip *per hop*, one at a time:
232.9 ms / 2 million hops ≈ 116 ns each — the section-1 number, measured by you.

That is the whole physics of this lesson. Everything from here on is the same law
showing up in ordinary C++ code, five times, each with a drill.

### 5. Consequence 1 — contiguous beats pointer-chasing: `vector` vs `list`

First, a fact about C++ you already own from lessons 01-02: containers hold
**values, in place**. **Contiguous** means "in one unbroken block of memory,
element after element" — and `std::vector<int>` is exactly that: one heap block,
the ints themselves back to back, 32 of them per 128-byte line. Nothing sits
between your loop and the bytes.

`std::list<int>` is the opposite design: a **doubly-linked list**. Every element
is a *separate heap allocation* holding `{prev pointer, next pointer, value}` —
roughly 24 bytes to store 4 — and advancing to the next node means loading a
pointer and following it. That is precisely the dependent load from section 4.

```cpp
std::vector<int> vec(10'000'000);            // fill with i % 256
std::list<int>   lst(vec.begin(), vec.end());

long long sv = 0; for (int v : vec) sv += v;   // 0.4 ms   on one machine, -O2
long long sl = 0; for (int v : lst) sl += v;   // 8.0 ms   same values: ~19x
```

Identical loop bodies, identical sums, ~19x — **and this is the friendly case**:
the list was built in one go, so its nodes came out of the allocator nearly
adjacent. In a long-running program that inserts and erases, the nodes scatter,
and the walk drifts toward the 460x chase of section 4.

You have been running both of these layouts for years without the names. A NumPy
`ndarray` is the `vector` side — python drill 08 defined it as "a single fixed-size
block of raw memory," which is precisely `std::vector`'s layout: one contiguous C
buffer of machine values. A Python `list` is the other side: under the hood it is an
array of pointers, each element a separate heap-allocated Python object — even
`[1, 2, 3]` stores three pointers to three `int` objects placed wherever the
allocator liked, so walking a list is pointer-chasing by construction. The NumPy
lesson measured the pure-Python loop at well over 100x slower than the vectorized
one; much of that is interpreter overhead, but a solid chunk is the measurement you
just made — a pointer hop per element versus a contiguous stream. "Vectorize" was
cache advice all along: hand the loop to code that walks contiguous memory.

*Why does the language even offer `std::list`?* Because its structural promises
are real: inserting in the middle is O(1) *once you are standing there*, and
nodes never move, so pointers to elements stay valid. The catch the textbook
skips: you must *walk* to the middle first — a pointer chase — while `vector`'s
"expensive" O(n) shuffle-down is one streaming pass over contiguous memory, the
pattern hardware loves most. This is where the interview quote earns its keep.

*Where you'll meet it:* every per-frame collection in a robotics pipeline —
detections, point clouds, joint trajectories — is a contiguous buffer. The rule:
**default to `vector`; demand measured evidence before using anything
node-based.**

### 6. Consequence 2 — walk the matrix the way it is stored (mystery solved)

A 2D image or matrix in C++ is normally one flat contiguous block in **row-major**
order: all of row 0, then all of row 1, and so on, with element `(r, c)` at index
`r * cols + c`. (The transposed convention — column-major — exists too, in Fortran,
MATLAB, and some BLAS libraries.) The flat block is also how camera frames and
tensors arrive from every real API, so `m[r * cols + c]` is a pattern your fingers
should know.

Now re-read the two loops from the top of this lesson, with section 3-4 eyes:

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

The first loop touches addresses 0, 4, 8, ... — every byte of every cache line
gets used, and the prefetcher streams lines ahead of the loop. The second hops
16 KB per step (one full row), uses 4 bytes of each line it drags in, and by the
time it wraps around to that line's neighbors, the line has long been evicted —
so the 64 MB gets re-fetched over and over. Mystery solved: the 6x was never in
the arithmetic; it was in the *order of addresses*.

Row-major is also NumPy's default — the flag is literally called "C order".
`np.zeros((rows, cols))` is one flat buffer with element `(r, c)` at
`r * cols + c`; `arr[r, c]` does this same index arithmetic for you. You have been
writing row-major code since python drill 08 — C++ just stops hiding the
multiplication.

The rule costs nothing to apply: **make the last index the innermost loop**. For
images (row-major) that means `y` outer, `x` inner — burn
`for (y) for (x) pixel[y * width + x]` into your fingers. The same decision at
framework scale is tensor layout: NCHW vs NHWC is "which dimension sits
contiguous in memory," chosen so the hot kernel walks stride-1. Hold that
thought — it returns in the road-ahead section.

### 7. Consequence 3 — struct padding: smaller structs, more per line

Sections 5-6 were about the *order* you touch bytes. The next lever is how many
useful bytes each cache line even *contains* — which turns out to be something
you control by how you declare a struct.

Two compile-time operators first. **`sizeof(T)`** gives the size of a type in
bytes. **`alignof(T)`** gives its alignment (defined next). On every mainstream
64-bit platform: `char` is 1 byte, `int` and `float` are 4, `double`, `long long`,
and every pointer are 8.

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
  `double`, N is 8; for `int`, 4; for `char`, 1 (`alignof(double) == 8`). The rule
  exists because of *hardware*: the memory system reads at aligned addresses in
  aligned units, so an aligned load is one simple operation, while a misaligned
  one can straddle two units — slower on some chips, illegal on others. C++ bakes
  the rule in so every ordinary load is the fast kind.
- **Padding**: unused filler bytes the compiler inserts so every field lands on
  its alignment. The struct's total size also rounds up to a multiple of its
  largest field alignment, so that in an array `element[1]` starts aligned too.

One more C++ rule completes the puzzle: **the compiler must keep your declaration
order** (a compatibility guarantee inherited from C, where structs describe wire
formats and hardware registers byte-for-byte). It may only insert padding, never
reorder. So the packing job is yours. Sort fields largest-alignment-first and the
padding melts away:

```cpp
struct GoodOrder {     // total: 16 bytes — same four fields, 33% smaller
    double timestamp;  // offset 0
    int    id;         // offset 8
    char   ready;      // offset 12
    char   flags;      // offset 13   <- 2 tail-pad bytes round 14 up to 16
};
// Verified with clang++ -std=c++20 -Wall -Wextra -Werror=return-type -O2:
//   sizeof(BadOrder) == 24, sizeof(GoodOrder) == 16
// Smaller demo of the same rule:
//   struct {char a; int b; char c;}  -> 12 bytes
//   struct {int b; char a; char c;}  ->  8 bytes
```

Why care about 8 bytes? Multiply by a million detections per minute of video, and
remember the currency of section 3: smaller structs mean more elements per cache
line, which means fewer DRAM trips for the same loop.

NumPy users have already met `sizeof` under another name: **itemsize**.
`np.dtype(np.float32).itemsize` is 4 — that is `sizeof(float)` — and a *structured*
dtype replays this entire section. Verified with this repo's NumPy (2.4.6):
`np.dtype([('ready','u1'), ('timestamp','f8'), ('flags','u1'), ('id','i4')])` has
itemsize 14, because NumPy packs tightly by default — but pass `align=True` and it
pads to 24, exactly `sizeof(BadOrder)`, by exactly these alignment rules. Put the
fields largest-first and the aligned itemsize is 16, exactly `sizeof(GoodOrder)`. A
C++ struct is an aligned dtype you cannot switch off — which is why the packing job
falls to you.

*Where you'll meet it:* message structs on the wire to a robot arm (CAN and serial
protocols are packed byte layouts), shared-memory frames between a vision process
and a controller, and every "why is `sizeof` 24 and not 14?" interview whiteboard.

### 8. Consequence 4 — AoS vs SoA: which fields travel together?

Say your detector emits a struct per object (six 4-byte fields — alignment 4, so
`sizeof(Detection) == 24` with zero padding; predict it yourself using section 7):

```cpp
struct Detection { float x, y, w, h; float score; int class_id; };
```

- **AoS — array of structs**: `std::vector<Detection>` — whole records side by
  side: `x y w h score id | x y w h score id | ...`. Natural to write; each
  element's fields are together.
- **SoA — struct of arrays**: one packed array per field: `std::vector<float>
  scores; std::vector<float> xs; ...` — all scores side by side:
  `score score score ...`.

You have felt this choice in Python. A list of dicts —
`[{'x': ..., 'score': ...}, ...]` — is AoS (with extra pointer-chasing on top); a
dict of NumPy arrays — `{'xs': np.array(...), 'scores': np.array(...)}` — is SoA,
and so is a pandas DataFrame, which stores each column as its own contiguous array.
If you have ever converted a list of dicts into arrays and watched a scan get
dramatically faster, this section is the receipt.

One piece of hardware completes the picture. **SIMD** (single instruction, multiple
data — "vector instructions") lets one CPU instruction process 4-16 values at once —
but only when those values sit next to each other in memory. Keep that in mind while
reading the measurement.

Now scan 20 million detections for the best score. AoS reads 4 useful bytes per
24 — it drags 480 MB through the cache to use 80 MB. SoA reads exactly the 80 MB
it needs. Measured honestly, in two acts (one machine, 20M detections):

```cpp
float best = -1.0f;
for (const Detection& d : aos) best = d.score > best ? d.score : best; // 9.6 ms
for (float s        : scores)  best = s      > best ? s      : best;   // 9.4 ms
// Plain -O2: barely different! Each max depends on the previous one — a
// DEPENDENCY CHAIN. The CPU can't start iteration N+1's max until N's is
// done, so the *core* is the bottleneck and the memory difference hides.

// Compile the same code with -O2 -ffast-math (lets the compiler reorder
// float arithmetic, which unlocks SIMD for reductions like max and sum):
//   AoS: 5.2 ms          SoA: 0.9 ms          — ~6x, on one machine
```

The packed `scores` array vectorizes — SIMD eats it 4-16 floats at a time; the
stride-24 walk through `Detection`s cannot. That is the full lesson of SoA:
**layout doesn't just set your cache traffic — it decides whether SIMD is possible
at all.** Inference runtimes are SoA from top to bottom; a tensor is SoA taken to
its logical extreme — and it is the deeper reason NumPy's whole-array operations
win: they run compiled loops over packed same-type values, the one layout the
hardware can process several-at-a-time.

AoS is still right when the hot loop uses *all* fields of one element together
(box math in NMS, integrating one robot state). The design question to ask out
loud in an interview: **"which fields travel together through the hot loop?"**
Fields that travel together, stay together; a field scanned alone earns its own
array.

### 9. Consequence 5 — `reserve()`: don't reallocate in the middle of a hot loop

One consequence left, and it moves from *reading* buffers to *building* them.

A `std::vector` owns one heap block. `size()` is how many elements you have;
**capacity** is how many fit in the current block. `push_back` onto a full vector
triggers a **reallocation**: allocate a bigger block, copy every element over,
free the old block — and every pointer and iterator into the old block is now
invalid (lesson 02's dangling pointers, again). Growth is geometric — libc++
doubles:

```cpp
std::vector<int> v;                  // capacity 0 — a fresh vector owns no memory
for (int i = 0; i < 1'000'000; ++i) v.push_back(i);
// capacity walks 1, 2, 4, 8, ..., 1048576: 21 reallocations (measured),
// recopying everything each time — ~2 million element copies total.

std::vector<int> w;
w.reserve(1'000'000);                // ONE allocation, up front
for (int i = 0; i < 1'000'000; ++i) w.push_back(i);
// capacity changes exactly once: 0 -> 1000000 at the reserve (measured).
// 10M push_backs, one machine: 6.9 ms without reserve, 2.4 ms with (~3x) —
// and, often more important in robotics, no mid-loop allocation spikes and
// no surprise pointer invalidation.
```

*Why does `vector` grow this way at all?* Doubling makes `push_back` cheap *on
average* (each element is recopied a bounded number of times — "amortized O(1)"
in interview language), which is the right default for code that does not know
its final size. Python's `list.append` plays the same amortized game under the
hood — geometric growth, periodic recopying — you have simply never had a
`reserve()` to reach for. But a 30-60 Hz control loop *does* know its size, cannot
afford a surprise multi-millisecond allocation spike mid-frame, and really cannot
afford a stale pointer into a freed block. `reserve()` is one line that deletes
both risks.

### 10. Why C++ hands you this control

Step back. Nothing in sections 5-9 changed an algorithm — every win came from
choosing *where bytes sit*. Python is the clearest contrast: it does not let you
make these choices. Every object lives behind a pointer, placed wherever the
allocator liked; a list of a million floats is a million pointer-chases. That is a
perfectly good design for productivity — and it takes the performance lever out of
your hands, because as this lesson measured, **layout IS the performance lever** on
modern hardware. It is also exactly why NumPy exists: an ndarray is C's memory
layout — contiguous, typed, padding-predictable — smuggled into Python, with the
hot loops compiled. Every time you vectorized, you were renting this lesson's
discipline. C++ hands you the deed.

C++'s philosophy is the opposite bet, and it is the same zero-overhead philosophy
you have met in every lesson so far: *the programmer, not a runtime, decides.* A
struct is exactly the bytes you declare (plus padding you can predict and
control). A local really lives on the stack. A `std::vector<Pose>` is one solid
slab of 12-byte records shoulder to shoulder. When an interviewer asks "why is
C++ the language of inference and robotics?", this is the answer: at 30-60 frames
per second, cache-friendly vs cache-hostile is the difference between hitting the
control deadline and missing it — and C++ is the language that lets you choose.

The rules this lesson leaves you with:

1. **Default to `std::vector`.** Contiguity is the whole ballgame (sections 4-5).
2. **`reserve()` when you know the size** — before any big `push_back` loop.
3. **Keep hot structs small** and order fields largest-alignment-first (section 7).
4. **Iterate in storage order** — last index innermost; `y` outer for images.
5. **Split hot fields from cold (SoA)** when a loop scans one field across many
   elements (section 8).
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
a whiteboard (you can check your paper answer in NumPy first: section 7's
`align=True` structured dtype replays both numbers). For real: message structs on
the wire to a robot arm (CAN/serial protocols are packed byte layouts),
shared-memory frames between a vision process and a controller, and shrinking a
per-detection struct so more of them fit per cache line in the post-processing loop.

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
never — and *why*), and the Big-O-vs-locality discussion this whole lesson is named
after: O(1) middle-insert still has to *walk* to the middle, one dependent load per
node. You have measured this gap once before, from the other side: it is the C++
half of the list-vs-ndarray comparison in python drill 08. For real: point clouds,
detection buffers, joint trajectories — every per-frame collection in a robotics
pipeline is a contiguous buffer, and node-based structures inside a control loop
are a code-review rejection.

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
tensors exist at all — a tensor is SoA taken to the limit, and a DataFrame is SoA
wearing a spreadsheet costume.

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
clang++ -std=c++20 -Wall -Wextra -Werror=return-type -o /tmp/mem starter.cpp && /tmp/mem

# The tests compile without optimization (correctness only). For timing numbers
# worth reading — like the ones quoted above — turn the optimizer on:
clang++ -std=c++20 -Wall -Wextra -Werror=return-type -O2 -o /tmp/mem solution.cpp && /tmp/mem
```

## The road ahead

Everything in this lesson was practice for the GPU — the ideas transfer almost
word for word, which is why this lesson matters more for your CUDA future than
any other in the track.

- **Memory coalescing is the cache line at warp scale.** A GPU runs threads in
  groups of 32 called *warps*, and when the 32 threads of a warp read 32
  *adjacent* addresses, the hardware merges ("coalesces") them into a few wide
  memory transactions — one box from the warehouse, all of it used. Scattered
  addresses shatter into many transactions, and the kernel crawls. It is
  section 3's law — memory moves in chunks; use the whole chunk — with 32
  workers instead of one.
- **AoS vs SoA is THE GPU data-layout question.** On the GPU, "which fields
  travel together" stops being a refinement and becomes the whole design: thread
  `i` reading `scores[i]` from a packed SoA array is perfectly coalesced, while
  thread `i` reading `dets[i].score` strides 24 bytes and wastes most of every
  transaction. CUDA code is SoA by default for exactly the reason section 8
  measured — the layout decides whether the memory system can be used at all.
- **NCHW vs NHWC is this exact concept, shipped.** Deep-learning frameworks argue
  about tensor layout because a convolution kernel wants its innermost loop
  walking stride-1 memory — section 6's "last index innermost", decided once for
  every kernel in the framework. When you read a cuDNN or TensorRT doc that says
  a layout is "faster on tensor cores", you now know what sentence is hiding
  underneath.

When you get to CUDA, you will also meet *shared memory* — a small, fast,
programmer-managed stash on each GPU multiprocessor. You already know what it is:
the kitchen counter from section 2, except this time *you* decide what goes on
it. Every instinct this lesson built — walk contiguously, use whole chunks, keep
hot data small and together — is the instinct that makes GPU kernels fast.
