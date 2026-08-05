# 07 — Containers and Contiguous Storage

This lesson teaches `std::vector<T>` — the container behind most C++ code you
will ever read — through the lens of what it physically is: one contiguous
block of memory that the vector owns. After it, you can build a grayscale
camera frame as `width * height` bytes in a single constructor call, sum it
through a `const` reference, and — the part that prevents real bugs — say
exactly when a pointer into a vector dies. Those first two are the drills:
`make_gray_frame` and `checksum`. Every output shown in a comment was run and
verified.

## The problem this lesson solves

In Python you never asked where a list's elements lived; the interpreter hid
storage completely, and the same list happily held ints, strings, and other
lists. C++ makes storage your business, and the payoff is why robotics and
inference code is written in it: an image is a flat run of bytes, a tensor a
flat run of floats, and the hardware chews through contiguous memory orders of
magnitude faster than it chases scattered references. Not knowing the storage
model produces two characteristic failures: element-by-element construction
loops where one constructor call would do, and — much worse — holding a
pointer into a vector across a `push_back` and reading memory that has been
freed.

## The lesson

### A vector owns one contiguous block

A **`std::vector<T>`** is a resizable sequence whose elements all have the
type `T` named in the angle brackets. Contrast this with the Python list you
know: a Python list is an array of *references*, each slot pointing at an
object boxed somewhere else in memory — which is why `[1, "two", 3.0]` is
legal. A vector stores the elements *themselves*, packed side by side in one
**contiguous** (gap-free, single-block) run of memory, so
`std::vector<std::uint8_t>` is literally a run of bytes. Two types from the
starter's signatures, defined once: **`std::uint8_t`** is an integer of
exactly 8 bits (0–255 — one grayscale pixel), and **`std::size_t`** is the
unsigned integer type C++ uses for sizes and indices.

The vector object itself is small — it records where the block lives, how
many elements are in use, and how much room remains — and it **owns** the
block: destroy the vector and the memory is freed with it, deterministically,
no garbage collector involved.

### Two constructors that look almost identical

```cpp
std::vector<int> a(3, 7);   // (count, fill-value)  -> size 3: {7, 7, 7}
std::vector<int> b{3, 7};   // literal contents     -> size 2: {3, 7}
```

Parentheses select the **fill constructor** — "this many elements, all set to
this value" — allocating the whole block in one shot. Braces list the actual
contents, exactly like a Python list literal. Mixing them up is silent and
wrong by a lot; `make_gray_frame` is the parenthesis form wearing a domain
name.

### Bytes print as characters

`size()` counts elements; `front()` and `back()` are the first and last
elements (the starter's asserts use all three). The trap unique to byte
vectors: `std::uint8_t` is an alias for `unsigned char`, and streams print
chars as *text*:

```cpp
std::vector<std::uint8_t> v(3, 65);
std::cout << v[0];        // -> A     streamed as a character — 65 is 'A'
std::cout << int(v[0]);   // -> 65    cast to int to see the number
```

Print `int(pixel)` whenever you are debugging pixel values, or the console
will show you letters and bell noises.

### Contiguity is the payoff: `data()` and flat images

**`data()`** returns a pointer (lesson 05) to element 0. Because the block is
contiguous, pointer arithmetic is element arithmetic — and a 2-D image can
live in a 1-D vector via the **row-major** rule: row 0 first, pixel
`(row, col)` at index `row * width + col`.

```cpp
std::vector<std::uint8_t> frame(12, 7);   // a 4-wide, 3-tall image, row 0 first
const std::uint8_t* px = frame.data();
int(px[1 * 4 + 2])                        // -> 7     pixel at row 1, column 2
frame.data() + frame.size() == &frame.back() + 1   // -> true
```

That last expression is the starter's final assert: `data() + size()` is the
**one-past-the-end** position — a valid address to compute and compare, never
to read through. Every image and tensor library flattens 2-D data exactly
this way.

### Growth moves the block — and kills your pointers

`push_back(x)` appends one element. **`capacity()`** is how many elements the
current block can hold; when a `push_back` would push `size()` past it, the
vector **reallocates**: allocate a bigger block, copy every element over,
free the old block. Watch it happen:

```cpp
std::vector<std::uint8_t> v;
// push_back nine times, printing whenever capacity changes:
// size 1 -> capacity 1
// size 2 -> capacity 2
// size 3 -> capacity 4
// size 5 -> capacity 8
// size 9 -> capacity 16    (this library doubles; exact numbers vary elsewhere)
```

The consequence is the sharpest trap in this module — every pointer,
reference, and iterator into the old block now dangles:

```cpp
std::vector<std::uint8_t> w(4, 7);   // size 4, capacity 4 — completely full
const std::uint8_t* p = w.data();
w.push_back(7);                      // no room: reallocate, copy, free old block
p == w.data()                        // -> false   p still holds the OLD address
```

Reading through `p` now is undefined behavior — lesson 05's dangling borrow,
triggered by the container instead of a closing scope. The rule: never hold a
pointer, reference, or iterator across anything that can grow the vector;
re-fetch from the vector instead.

### Copies are real copies

The lesson-03/04 story, scaled up. In Python, `v = u` aliases and you request
a copy explicitly; in C++ assignment and initialization copy — *all* the
elements, into a new independently owned block:

```cpp
auto copy = frame;    // copies every byte into fresh storage
copy[0] = 0;
int(frame[0])         // -> 7    the original is untouched
```

This is also why `checksum` takes `const std::vector<std::uint8_t>&`
(lesson 04): copying a megapixel frame on every call is real, measurable
cost, and the reference makes it free.

## Muscle memory

Type these until they come out without thinking:

```cpp
std::vector<std::uint8_t> frame(w * h, fill);  // fill constructor: PARENS, not braces
frame.size()                                   // element count (std::size_t)
frame[row * width + col]                       // row-major pixel access
int(frame[i])                                  // print a byte as a number
frame.data() + frame.size()                    // one past the end — compare, never read
long long total = 0;
for (std::uint8_t p : frame) total += p;       // accumulate bytes in a WIDER type
```

## The drills

Work through `starter.cpp` top to bottom: one builder, one reader.

### `make_gray_frame(width, height, fill)`

Return a vector of `width * height` pixels, every one initialized to `fill`.

```cpp
const auto frame = make_gray_frame(4, 3, 7);
frame.size()     // -> 12    4 * 3 pixels
frame.front()    // -> 7     first pixel is fill...
frame.back()     // -> 7     ...and so is the last
```

One line: `return std::vector<std::uint8_t>(width * height, fill);`. The trap
is the constructor pair from the lesson — braces, `{width * height, fill}`,
would build a *two-element* vector and fail the `size() == 12` assert on the
spot. The tests also re-check the contiguity fact: `frame.data() +
frame.size()` must equal `&frame.back() + 1`, which the fill constructor
gives you for free.

**Where you'll see it:** allocating a buffer before filling it is the first
line of every image pipeline — a camera adapter allocates the frame, then the
driver writes into it — and inference runtimes allocate input and output
tensors the same way (`std::vector<float>(n, 0.0f)`). In interviews it is the
setup line of half of all dynamic-programming and graph problems:
`std::vector<int> dist(n, INF)` for shortest paths, `std::vector<int>
counts(256, 0)` for counting sort.

### `checksum(pixels)`

Add every pixel into a `long long` total, through the read-only reference.

```cpp
checksum(make_gray_frame(4, 3, 7))   // -> 84    12 pixels x 7
```

Shape: a range-for accumulating into a total. Copying each element is fine
here — a byte is cheaper to copy than to alias — but the *accumulator's* type
is the trap the tests probe. A frame of bytes sums far past 255, and byte
arithmetic wraps around silently:

```cpp
std::uint8_t total = 0;
total += 200;
total += 100;    // 300 does not fit in 8 bits
int(total)       // -> 44    (300 mod 256 — no error, no warning)
```

Declare `long long total = 0;` and the sum has room for billions of pixels.
The parameter is `const std::vector<std::uint8_t>&` — lesson 04's
look-don't-touch — so the frame is neither copied nor modifiable.

**Where you'll see it:** checksums catch dropped and corrupted frames at
every hardware boundary — camera to driver, driver to pipeline, host to
accelerator in an inference stack. The wider-accumulator rule generalizes to
every reduction you will ever write: summing pixels, audio samples, or token
counts into a too-narrow type is a classic silent-overflow bug, and "what's
wrong with this sum?" is a stock interview probe.

## How to practice

Write your attempts in `starter.cpp`, then run the tests against them:

```bash
PRACTICE=1 uv run pytest ramp_up/cpp/01_stl_containers/lessons/07_containers_and_storage -q
```

Drop the `PRACTICE=1` to run them against the reference `solution.cpp`.
