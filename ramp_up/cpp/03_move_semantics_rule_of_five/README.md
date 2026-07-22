# 03 — Move Semantics and the Rule of Five

After this lesson you will be able to predict — and prove with counters — whether a
given line of C++ copies an object or steals its guts, explain what `std::move`
actually compiles to (nothing), write the five special member functions for a
resource-owning class without forgetting the `noexcept`, say why the modern answer is
usually to write *none* of them, and stop "optimizing" return statements that were
already free. This is the single most-asked senior C++ interview topic: "what does
`std::move` do?", "write a Rule-of-Five class", and "what state is a moved-from object
in?" are asked practically verbatim.

Lesson 02 introduced ownership, destructors, and `std::move` for `unique_ptr` — a type
that *bans* copying, so moving was the only way anything traveled. This lesson is about
the rest of the world: types like `std::vector` that can *both* copy and move, where
every call site silently picks one, and picking wrong in a hot loop is the difference
between shuffling three pointers and shoveling megabytes.

## The Java you know

In Java, a variable of object type holds a **reference** — an address pointing at the
object (lesson 02) — and `=` copies the *reference*, never the object:

```java
int[] a = new int[1_000_000];           // ~4 MB of data on the heap
int[] b = a;                            // copies ONE reference: 8 bytes, instant
b[0] = 42;                              // visible through a — same object, two names
int[] c = Arrays.copyOf(a, a.length);   // duplication is always explicit and loud
```

Eight years of Java have taught you three reflexes, and this lesson flips two of them
(lesson 02 already flipped the third):

- **Passing, returning, and storing objects is always cheap** — you were only ever
  moving 8-byte references around. *Flipped below.*
- **Duplicating an object is rare and explicit** — `clone()`, `Arrays.copyOf`, copy
  factories. Nothing gets duplicated by accident. *Flipped below.*
- **Nobody owns the object** — the GC frees it eventually. *Flipped in lesson 02:
  in C++ every object has exactly one owner, and the owner's destructor frees it.*

## The lesson

### 1. In C++, `=` copies the whole object

The ground fact this entire lesson stands on: in Java, assignment copies a
**reference**; in C++, assignment copies **the object**. All of it.

```cpp
std::vector<int> a(1'000'000, 7);   // one million ints, all 7  (digit separator: ')
std::vector<int> b = a;             // copies ALL one million ints. Right here.
b[0] = 42;
// a[0] == 7   b[0] == 42          <- verified: two INDEPENDENT objects
// a.size() == 1000000  b.size() == 1000000
```

That copy is a **deep copy**: `b` gets its own freshly allocated heap buffer and every
element is duplicated into it, so afterwards `a` and `b` share nothing. (The opposite —
copying just the pointer so both objects point at one buffer — is called a **shallow
copy**, and in a language with destructors it is a bug factory: two owners, one buffer,
two frees. That's the double-free from lesson 02.)

The machinery: when you write `std::vector<int> b = a;`, the compiler calls `vector`'s
**copy constructor** — the function that builds a *new* object as a duplicate of an
existing one. When you assign into a vector that *already exists* (`b = a;` on a later
line), it calls the **copy assignment operator** instead — same duplication, but it
also has to release whatever `b` was holding before. Two different functions; you'll
write both in the drill.

This whole-object behavior is called **value semantics** — variables *are* objects,
not references to objects — and it's the #1 mental shift coming from Java
([`../LEARNING_POINTS.md`](../LEARNING_POINTS.md) §1). It has real upsides (no aliasing
surprises: nobody else can mutate your vector behind your back). This lesson is about
the downside and its cure.

### 2. Why the default hurts: a 2 MB frame, 30 times a second

Put real numbers on it. One grayscale full-HD camera frame is 1920 × 1080 = 2,073,600
bytes — call it 2 MB. Your robot's camera delivers 30 of them per second. Now look at
an innocent frame-processing loop written with Java reflexes:

```cpp
std::vector<unsigned char> frame = grab_frame();   // ~2 MB of pixels
pending.push_back(frame);       // Java brain: "stored a reference, done"
                                // C++ reality: deep-copied all 2,073,600 bytes
```

You can *prove* the copy happened without any special tooling — independence is the
tell:

```cpp
frame[0] = 255;                 // scribble on the original AFTER the push_back
// pending.back()[0] is unchanged  <- verified: the stored frame is a private copy
```

At 30 fps that reflex quietly burns ~62 MB of memory traffic per second — per camera,
per queue hop — before the model has done a single multiply. In an inference pipeline
(capture → preprocess → inference → publish), a frame that crosses four such
boundaries gets duplicated four times. Latency budgets die exactly here, which is why
interviewers for robotics and inference roles keep poking at this topic.

So: copying is the default, copying is expensive, and yet the frame obviously has to
*get* from `grab_frame()` into `pending` somehow. What we want is a way to *hand the
buffer over* without duplicating it — Java's cheap handoff, but explicit and
compiler-checked. C++ has exactly that. Two small definitions first.

### 3. Temporaries: values that are already dying

Run this line in your head:

```cpp
std::string s = "hi";
std::string shout = s + "!!";   // s + "!!" builds a brand-new string... where?
```

`s + "!!"` has to produce a `std::string`, and that string is not stored in any
variable — it exists only inside the expression. Such a value is called a
**temporary**: an object the compiler creates to hold an intermediate result and
destroys at the end of the statement (at the semicolon). It has no name. You cannot
mention it on the next line. It is *already dying*.

C++ has formal words for this split, and interviewers use them, so learn them here:

- An **lvalue** is an expression that names a persistent object — something with a
  name you can refer to again on a later line. `s` is an lvalue; so is `frame`,
  so is `robot.name_`. (Mnemonic: historically the thing allowed on the **l**eft of
  `=`.)
- An **rvalue** is an expression whose value has no persistent home — a temporary,
  about to die. `s + "!!"` is an rvalue; so is `make_frame(4, 3)` — the function's
  return value, before it's stored anywhere. (Historically: only allowed on the
  **r**ight of `=`.)

Java never needed to teach you this distinction because copying a reference costs the
same 8 bytes either way. C++ cares intensely, because of one observation:

**Copying from a temporary is absurd.** The temporary is about to be destroyed. Nobody
can ever look at it again. So if initializing `shout` were to *steal* the temporary's
internal buffer instead of duplicating it — who could possibly tell the difference?

Nobody. That legal loophole is the entire foundation of move semantics.

### 4. Move: steal the guts of a dying object

A `std::vector` is internally just three machine words: a pointer to its heap buffer,
a size, and a capacity. Copying a vector means allocating a new buffer and duplicating
*n* elements — O(n). **Moving** a vector means copying those three words into the
destination and blanking them in the source — the new vector now owns the old buffer,
and the old vector owns nothing. O(1), regardless of whether the buffer holds ten
bytes or ten gigabytes. Watch it happen:

```cpp
std::vector<int> a(1'000'000, 7);
const int* before = a.data();        // address of a's heap buffer
std::vector<int> b = std::move(a);   // MOVE construction: b steals the buffer

// b.data() == before   <- verified: the SAME heap buffer, zero elements copied
// a.size() == 0        <- verified: a is now valid but EMPTY
// b.size() == 1000000
```

Unpack the two new things on that middle line:

- **`std::move` moves nothing.** It is a *cast* — a compile-time re-labeling that
  produces no instructions. `std::move(a)` means "treat `a` as an rvalue": *I,
  the programmer, promise I no longer need `a`'s contents — you may steal from it.*
  You met this exact sentence in lesson 02 with `unique_ptr`; it's true for every
  type. If nothing ends up stealing from `a`, nothing happens at all.
- The stealing is done by the **move constructor** — a constructor overload that
  takes `vector<int>&&`. The `&&` type is an **rvalue reference**: a reference that
  only binds to rvalues — genuine temporaries, or lvalues you've marked with
  `std::move`. Overload resolution does the routing: pass an lvalue, the copy
  constructor runs; pass an rvalue, the move constructor runs. Same call syntax,
  radically different cost.

The Java feeling closest to a move is `b = a; a = null;` — hand over the reference and
null out the old variable so no one uses it by mistake. But in Java that's a
convention you follow by hand; in C++ the handoff has dedicated syntax, its own
constructor, and O(1) guaranteed cost.

**What's left behind?** A moved-from standard-library object is **valid but
unspecified**: it's a real, un-corrupted object — safe to destroy, safe to assign a
new value into — but you must not assume anything about its contents. In practice a
moved-from `vector` or `string` is empty (and `vector`'s *move constructor* guarantees
the source ends up empty). For classes *you* write, you decide the moved-from state —
in the drill, a moved-from `FrameBuffer` is defined to be a 0×0 frame with no pixels,
and the asserts check it.

One calibration note so you don't over-apply the trick: moving only pays when the
object owns heap memory that can be handed over. `std::move` on an `int`, a `double`,
or a small struct of numbers just copies it — there are no guts to steal. (Fun
interview nugget: a short `std::string` like `"hi"` is stored *inline* inside the
string object — the "small string optimization" — so "moving" it copies those bytes
too.)

### 5. The Rule of Three, the Rule of Five, the Rule of Zero

Now flip from *using* moves to *providing* them, because `vector` already ships with a
correct copy constructor and move constructor — but the classes you write don't.

The compiler auto-generates six member functions if you don't write them; the five
that matter here are called the **special member functions** (the sixth is the default
constructor). For a class whose members are all self-cleaning (vectors, strings,
smart pointers), the generated ones are exactly right: generated copy copies each
member, generated move moves each member, generated destructor destroys each member.

The trouble starts when a class owns a resource *directly* — a raw `new[]` pointer, a
file descriptor, a CUDA buffer. The generated copy constructor copies members, and
copying a pointer member is a **shallow copy**: two objects now hold the same address,
both destructors free it, crash (lesson 02's double-free). Fixing that means writing
the copy operations yourself — hence the classic **Rule of Three**: *if a class needs
a hand-written destructor, copy constructor, or copy assignment operator, it almost
certainly needs all three* — they exist for the same reason (the resource), so
handling it in one place but not the others is a latent bug.

C++11 added moves, extending it to the **Rule of Five** — the full set:

```cpp
~T();                        // 1. destructor        release the resource
T(const T& other);           // 2. copy constructor   duplicate it (deep copy)
T& operator=(const T&);      // 3. copy assignment    duplicate into existing object
T(T&& other) noexcept;       // 4. move constructor   steal it
T& operator=(T&&) noexcept;  // 5. move assignment    steal into existing object
```

And here's which one a given line invokes — worth memorizing cold, interviewers ask
this as a lightning round:

```cpp
T b = a;             // 2: copy constructor   (new object born from an lvalue)
b = a;               // 3: copy assignment    (b already exists)
T c = std::move(a);  // 4: move constructor   (new object born from an rvalue)
c = std::move(b);    // 5: move assignment    (c already exists)
```

Three fine-print clauses, each of which is a named interview question:

**The silent-copy trap.** Declaring *any* of destructor / copy constructor / copy
assignment makes the compiler stop generating the move operations. Your class still
compiles, `std::move` still compiles — it just quietly *copies*:

```cpp
struct Logged {
    std::vector<int> data;
    ~Logged() {}                 // innocent-looking user-declared destructor...
};
Logged a;  a.data.resize(1000);
Logged b = std::move(a);         // ...so this "move" falls back to the COPY ctor
// a.data.size() == 1000   <- verified: source untouched — it was copied, not moved
```

No warning, no error — just every "move" of that type silently costing O(n). This is
the strongest argument for the counters in the drill: they turn an invisible
performance bug into a failing assert.

**The `noexcept` clause.** `noexcept` on a function is a promise it will not throw an
exception (Java analogy: an inverted `throws` clause — "throws nothing"; if the
promise is broken the program terminates rather than unwinding). Move constructors
must make this promise, for a concrete reason: when a `vector<T>` grows, it relocates
existing elements into the new buffer, and it will only *move* them if `T`'s move
constructor is `noexcept` — otherwise it *copies* every element, to guarantee it can
roll back intact if an exception strikes mid-relocation. Measured with the drill's
counters, relocating 2 elements while pushing a 3rd:

```cpp
// move ctor noexcept:      1 copy (the new element) + 2 moves   <- verified
// move ctor NOT noexcept:  3 copies, 0 moves                    <- verified
```

Forget one keyword, and every reallocation of every `vector<FrameBuffer>` in the
program degrades to deep copies. Silently.

**The Rule of Zero.** The modern punchline, previewed in lesson 02: since generated
members do the right thing when every field is self-cleaning, *hold resources through
`vector` / `string` / `unique_ptr` members and write none of the five.* Rule-of-Zero
classes make up ~95% of real code; hand-written Rule-of-Five classes are reserved for
the low-level 5% that wrap a raw resource (or need observable copy/move behavior —
which is precisely why the drill's instrumented `FrameBuffer` writes them out).

### 6. Returning by value is free — don't "help"

One habit from steps 1–5 would be exactly wrong, so it gets its own step. If copying
is expensive and `std::move` prevents copies, surely returning a big object should be
`return std::move(result);`?

No — and it's the opposite of harmless. Returning by value is already free:

```cpp
FrameBuffer make_frame(int width, int height) {
    return FrameBuffer(width, height);   // constructed DIRECTLY in the caller's variable
}
FrameBuffer f = make_frame(1920, 1080);  // copies: 0, moves: 0   <- verified by counters
```

This is **copy elision** ("elide" = skip entirely): the compiler builds the return
value straight into the caller's storage, so there is nothing to copy *or* move. When
the returned thing is a nameless temporary, as above, C++17 doesn't just permit this —
it *guarantees* it. Returning a *named* local goes through the same door (**NRVO**,
Named Return Value Optimization — "RVO" is the family name you'll hear in interviews):
not formally guaranteed, but applied by every mainstream compiler (verified: 0 copies,
0 moves on clang), and even where it can't be, a returned local is *moved*, never
copied — the language rules say a `return`ed local is treated as an rvalue first.

Which is why `return std::move(local);` is an anti-pattern with a name —
**pessimizing move**: wrapping the local in `std::move` changes the returned
expression's type in a way that *disqualifies* it from elision, so you pay a move
where you could have paid nothing. The compiler will tell you (verified, from
`clang++ -Wall` on exactly this code):

```text
warning: moving a local object in a return statement prevents copy elision
[-Wpessimizing-move]
```

Rule: `std::move` is for handing a named object *into* something (a container, a sink
parameter, another owner) — never for `return`ing a local. You saw the same rule for
`unique_ptr` in lesson 02; now you know the machinery behind it.

## Muscle memory

Type these until they require no thought:

```cpp
FrameBuffer(FrameBuffer&& other) noexcept;              // move ctor: && parameter + noexcept, always
frames.push_back(std::move(fb));                        // "done with fb — steal it"; fb now empty
FrameBuffer(const FrameBuffer& o) : data_(o.data_) {}   // copy ctor: deep copy via member init list
if (this != &other) { ... } return *this;               // assignment operator skeleton, self-assign guard
FrameBuffer make() { return FrameBuffer(w, h); }        // return by value — NEVER return std::move(x)
void store(FrameBuffer fb) { last_ = std::move(fb); }   // sink parameter: by value, then move into place
T tmp = std::move(a); a = std::move(b); b = std::move(tmp);   // swap = exactly three moves
class Pipeline { std::vector<FrameBuffer> frames_; };   // Rule of Zero: write none of the five
```

## The drills

Work through `starter.cpp` top to bottom. `FrameBuffer` carries two static counters —
`copies_made` and `moves_made`, with a `reset_counters()` helper — bumped inside the
copy and move operations, so `main()`'s asserts *prove* which special member ran on
each line. No guessing, no profiler: if your move constructor secretly copies, a
counter assert fails.

### `FrameBuffer` — the Rule of Five, instrumented

A class owning one grayscale image: `width_`, `height_`, and a
`std::vector<unsigned char>` of pixels. The ordinary constructor is given; you write
the five special members. Copies must be deep and count themselves; moves must steal
the vector (`std::move(other.data_)`), leave the source as a valid 0×0 empty frame,
count themselves, and keep their `noexcept`. The asserts check all of it, including
that a copy allocates its *own* buffer while a move re-uses the *same* buffer address:

```cpp
frames.push_back(fb);              // copies_made 0 -> 1; fb untouched (12 pixels)
frames.push_back(std::move(fb));   // moves_made  0 -> 1; buffer address IDENTICAL;
                                   // fb now 0x0 with 0 pixels: valid but empty
```

Where you'll see it: "write a Rule-of-Five class" (a `String`, `Buffer`, or `Matrix`
that owns memory) is *the* canonical senior C++ whiteboard exercise, with "what does
`std::move` actually do?" and "what state is a moved-from object in?" as its
follow-ups. In the field this class *is* the camera frame handed between capture and
inference threads, the tensor wrapper around a CUDA buffer in TensorRT/ONNX Runtime
code, the point cloud snapshot — big blobs that must cross pipeline stages without
being duplicated.

### `make_frame(width, height)`

A factory returning a `FrameBuffer` by value — one line, and the lesson is what you
*don't* write: no `new` (lesson 02), no output parameter, no `std::move`.

```cpp
FrameBuffer frame = make_frame(8, 2);   // copies_made == 0 && moves_made == 0: elided
```

Where you'll see it: interviewers show a by-value factory and ask "how many copies
does this make?" — the answer they want is *zero, guaranteed since C++17*, with RVO
named out loud. Its evil twin, `return std::move(local);`, is a code-review staple.
Real code: every loader and builder returns by value — calibration tables, parsed
model configs, preprocessed input tensors.

### `consume(fb)` — the sink

Takes its parameter **by value** and returns the sum of all pixel bytes as
`long long`. By-value is the point — the "sink" idiom for arguments a function keeps
or uses up. One signature, and the *caller* picks the cost per call site:

```cpp
consume(frame);              // caller keeps frame  -> parameter is a copy   (1 copy)
consume(std::move(frame));   // caller is done      -> parameter moves in    (1 move)
consume(make_frame(3, 1));   // temporary           -> built in place        (0 and 0)
```

Where you'll see it: "how should this function take its argument?" is a standard C++
design probe, and *pass by value, then move into place* is the modern answer for
sink-like setters and constructors (vs. the old `const T&` + `T&&` overload pair —
being able to argue the trade-off is senior-signal). Real code: `queue.push(std::move(frame))`
handing a frame to the encoder or inference worker — moving is the only sane way to
send a big buffer to another thread, because after the handoff exactly one thread owns
it.

### `swap_frames(a, b)`

Exchange two frames using a temporary and exactly **three moves** — tmp steals `a`,
`a` steals `b`, `b` steals `tmp`. Zero copies, zero pixels touched; the counters
enforce it. This three-line dance is literally how `std::swap` is implemented.

```cpp
swap_frames(a, b);   // copies_made == 0 && moves_made == 3
```

Where you'll see it: "implement swap without copying" is a classic warm-up that
quietly tests whether move *assignment* (not just construction) is at your
fingertips, and it's the engine inside the copy-and-swap idiom for writing
exception-safe assignment operators. Real code: double-buffering — swap front/back
frame buffers at vsync, ping-pong input/output tensors between inference iterations —
swaps megabytes at O(1) a thousand times a second.

## How to practice

```bash
# Against the reference solution (should pass out of the box):
uv run pytest ramp_up/cpp/03_move_semantics_rule_of_five -v

# Against YOUR implementation in starter.cpp:
PRACTICE=1 uv run pytest ramp_up/cpp/03_move_semantics_rule_of_five -v

# Or compile and run directly:
clang++ -std=c++17 -Wall -o /tmp/moves starter.cpp && /tmp/moves
```

Deep dives referenced above: value vs reference semantics
([`../LEARNING_POINTS.md`](../LEARNING_POINTS.md) §1), the practical `std::move`
summary (§10), and ownership, destructors, and `unique_ptr` in
[`../02_ownership_and_raii/`](../02_ownership_and_raii/README.md).
