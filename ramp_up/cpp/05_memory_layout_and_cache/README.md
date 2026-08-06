# 05 — Memory Layout and Cache Behavior

This module turns memory-layout vocabulary into deterministic reasoning you can
use in inference and robotics code. The micro-lessons use object sizes, member
alignment, literal address orders, modeled cache-line indices, and capacity
changes. They do not grade machine-dependent timing ratios.

## What this module consumes

You will use Module 01's contiguous containers and Module 03's resource-aware
value types. In particular, you should already know that `std::vector<T>` owns
its elements and that moving, copying, and destroying an owner affects the
resource it manages.

## What this module produces

After the module, you should be able to reason predictably about the data path
for a frame, tensor plane, detection collection, or batch:

- distinguish an object's inline members from a vector's separate payload;
- calculate aligned member offsets and identify compiler-inserted padding;
- model useful bytes per cache line from an explicit access pattern;
- trace row-first and column-first access through row-major storage;
- choose AoS or SoA from the fields an operation consumes; and
- reserve vector capacity while explaining reallocation and invalidation.

## Diagnostic

Before opening a starter, answer these aloud:

1. A `Frame` contains dimensions and a pixel vector. Which bytes are counted by
   `sizeof(Frame)`, and where are the pixel bytes?
2. Given `char`, `double`, `char`, and `int` members in that order, where must
   padding appear on the target ABI? Which declaration order is denser?
3. In a 64-byte arithmetic line model with four-byte elements, how many line
   indices do 16 stride-1 accesses touch? What about stride 16?
4. What offsets does a 2-by-3 row-major buffer expose under row-first and
   column-first traversal?
5. If a maximum-score pass consumes only scores and winning IDs, when is SoA a
   better fit than AoS? What invariant links the columns?
6. What happens to a pointer into a vector when `push_back` triggers growth, and
   how does `reserve` change that construction path?

If any answer depends only on “it benchmarked faster,” return to the relevant
micro-lesson and derive the structural reason first.

## Micro-lessons

| Lesson | Concept | Application |
| --- | --- | --- |
| [01 — Storage and object layout](lessons/01_storage_and_object_layout/) | Inline object bytes differ from owned payload storage | Draw a frame and its pixel allocation. |
| [02 — Alignment and padding](lessons/02_alignment_and_padding/) | Alignment inserts predictable padding | Reorder detection metadata for density. |
| [03 — Cache lines and locality](lessons/03_cache_lines_and_locality/) | Access stride determines modeled line use | Compare packed and strided tensor reads. |
| [04 — Traversal and contiguous storage](lessons/04_traversal_and_contiguous_storage/) | Loop order maps to row-major offsets | Traverse an image in storage order. |
| [05 — AoS and SoA](lessons/05_aos_and_soa/) | Layout should match the fields a pass consumes | Scan point or detection columns. |
| [06 — Allocation and `reserve`](lessons/06_allocation_and_reserve/) | Reserved capacity removes known mid-build growth | Build a predictable frame batch. |

Run all six reference solutions with:

```bash
uv run pytest ramp_up/cpp/05_memory_layout_and_cache/lessons -q
```

Practice one starter with `PRACTICE=1` and that lesson's directory.

## Capstone

The module-root capstone keeps the existing five drill groups:

1. reorder identical struct fields and report padded sizes;
2. sum a row-major matrix in row-first and column-first order;
3. sum identical values in contiguous and node-based containers;
4. scan detection scores in AoS and SoA layouts; and
5. count vector capacity changes with and without `reserve`.

The capstone prints timing lines so you can observe your own machine. Those lines
are informational only: correctness depends on structural facts and computed
results, never an elapsed-time comparison.

Edit the module-root `starter.cpp`, then run:

```bash
PRACTICE=1 uv run pytest ramp_up/cpp/05_memory_layout_and_cache/test_solution.py -q
```

Run only the reference capstone with:

```bash
uv run pytest ramp_up/cpp/05_memory_layout_and_cache/test_solution.py -q
```

Previous: [Module 04 — Virtual functions and vtables](../04_virtual_functions_and_vtables/)
· Next: [Module 06 — Threads, atomics, and queues](../06_threads_atomics_queues/).
