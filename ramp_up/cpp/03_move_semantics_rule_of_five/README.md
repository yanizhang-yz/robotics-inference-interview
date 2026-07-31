# 03 — Move Semantics and the Rule of Five

After this lesson you will be able to predict — and prove with counters — whether a
given line of C++ copies an object or steals its guts, explain what `std::move`
actually compiles to (nothing), write the five special member functions for a
resource-owning class without forgetting the `noexcept`, say why the modern answer is
usually to write *none* of them, and stop "optimizing" return statements that were
already free. This is the single most-asked senior C++ interview topic: "what does
`std::move` do?", "write a Rule-of-Five class", and "what state is a moved-from object
in?" are asked practically verbatim. Every snippet below was compiled and its claims
checked with `clang++ -std=c++20 -Wall -Wextra -Werror=return-type`.

Lesson 02 introduced ownership, destructors, and `std::move` for `unique_ptr` — a type
that *bans* copying, so moving was the only way anything traveled. This lesson is about
the rest of the world: types like `std::vector` that can *both* copy and move, where
every call site silently picks one, and picking wrong in a hot loop is the difference
between shuffling three machine words and shoveling megabytes.

## The problem: a frame has to travel

Picture the robot's vision pipeline. The camera produces one grayscale full-HD frame:
1920 × 1080 = 2,073,600 bytes — call it 2 MB. It produces 30 of them every second.
And each frame has to *travel*: capture hands it to preprocessing, preprocessing hands
it to the model, the model's output gets published. Every one of those handoffs is an
assignment, a function argument, or a return value.

In Python, none of this would cost a thought. `frame2 = frame` copies nothing — it
binds a second name to the same object, and passing `frame` into a function or
returning it is the same non-event. Handing data around is free in Python *because
assignment never copies*; a copy only happens when you explicitly ask for one
(`list(a)`, `copy.deepcopy(a)`).

C++ starts from the opposite default — the ground rule from lesson 01: a variable *is*
its object, a box of bytes, not a name for one, and assignment copies the whole box.
Hold that rule against the pipeline and an uncomfortable question appears: does every
handoff duplicate two megabytes?

By default: yes. This lesson first measures that cost, then teaches the machinery C++
grew that Python never needed: a way to hand a value over *without* copying it — move
semantics. The goal, in one sentence: **a handoff as cheap as Python's assignment, but
with a single owner.** The drill's counters will let you prove, line by line, which
handoffs copy and which cost nothing.

## The lesson

### 1. Assignment copies the whole object — and the cost is real

Start from the lesson 01 fact, scaled up to a million elements:

```cpp
std::vector<int> a(1'000'000, 7);   // one million ints, all 7  (digit separator: ')
std::vector<int> b = a;             // copies ALL one million ints. Right here.
b[0] = 42;
// a[0] == 7   b[0] == 42          <- verified: two INDEPENDENT objects
// a.data() != b.data()            <- verified: two DIFFERENT heap buffers
```

Two things in that snippet deserve names — and the first is a name you already know
from Python.

First, this is a **deep copy**: `b` gets its own freshly allocated heap buffer, and
every element is duplicated into it. Afterwards `a` and `b` share nothing — that is
what the two different `data()` addresses prove. You met the shallow-vs-deep split in
Python (`list(xs)` copies one level, `copy.deepcopy` copies everything — Gotcha 9 in
[`../../python/LEARNING_POINTS.md`](../../python/LEARNING_POINTS.md)). In C++ the
stakes are higher: a **shallow copy** — duplicating just the pointer, so both objects
aim at one buffer — is, in a language with destructors, a bug factory: two owners,
one buffer, two frees. That is lesson 02's double-free.

Second, the mechanism. Python runs no hidden code on `b = a` — assignment is a name
binding, and copying is a function you call. In C++, assignment *is* copying, so the
copying code has to live somewhere — and it lives in two special functions of the
class. When `std::vector<int> b = a;` builds a *new* object as a duplicate of an
existing one, the compiler calls `vector`'s **copy constructor**. When you assign
into a vector that *already exists* (`b = a;` on some later line), it calls the
**copy assignment operator** instead: same duplication, but it must also release
whatever `b` was holding before. Two different functions. You will write both in the
drill.

Now put a stopwatch on it. Copying one 2 MB camera frame:

```cpp
std::vector<unsigned char> frame(1920 * 1080, 128);   // 2,073,600 bytes
std::vector<unsigned char> copy = frame;              // measured: ~150 microseconds
```

150 microseconds sounds small until you multiply. At 30 fps, one careless
`pending.push_back(frame)` in the capture loop burns ~62 MB of memory traffic per
second — per camera, per queue hop. A frame that crosses four pipeline boundaries gets
duplicated four times before the model has done a single multiply. Latency budgets die
exactly here, which is why interviewers for robotics and inference roles keep poking
at this topic.

**Why does C++ make copying the default, instead of sharing?** Predictability. Python
chose the sharing default, and you have paid its price: mutate a list through one
name and every alias sees the change — the `[[0] * 3] * 3` grid trap, where writing
to "one row" changes all three. If C++'s `b = a` silently made both names refer to
one object, `b[0] = 42` would change `a` too — the same spooky action at a distance,
except now in a language where that shared buffer must also be freed by exactly one
of them. C++ picks the opposite trade: what a variable holds is *its own*, always,
and sharing is something you must ask for explicitly (a pointer, a reference).
Explicit over implicit. The price of that predictability is that duplication — the
expensive thing — is what you get when you write nothing special. The rest of this
lesson is about paying that price only when you mean to.

### 2. Temporaries: values with no name

We know what a handoff costs. Before we can make it cheap, we need one piece of
vocabulary. Run this line in your head:

```cpp
std::string s = "hi";
std::string shout = s + "!!";   // s + "!!" builds a brand-new string... where?
```

`s + "!!"` has to produce a `std::string`, and that string is not stored in any
variable — it exists only inside the expression. (Python builds the same nameless
intermediate when you concatenate strings; you never think about it because the
garbage collector quietly sweeps it up.) In C++ such a value is called a
**temporary**: an object the compiler creates to hold an intermediate result and
destroys at the end of the statement, at the semicolon. It has no name. You cannot
mention it on the next line. It is *already dying*.

C++ has formal words for this split, and interviewers use them, so learn them here:

- An **lvalue** is an expression that names a persistent object — something you can
  refer to again on a later line. `s` is an lvalue; so is `frame`, so is
  `robot.name_`. (Mnemonic: historically, the thing allowed on the **l**eft of `=`.)
- An **rvalue** is an expression whose value has no persistent home — a temporary,
  about to die. `s + "!!"` is an rvalue; so is `make_frame(4, 3)` — the return value
  of a function call, before it is stored anywhere. (Historically: only allowed on
  the **r**ight of `=`.)

Keep the distinction warm. The entire trick of this lesson rests on it.

### 3. The insight: stealing from the dying is safe

Why do temporaries matter? Because of what is inside a vector. Whatever its element
count, the vector object itself is just three machine words — a pointer to its heap
buffer, a size, and a capacity (verified: `sizeof(std::vector<int>)` is 24 bytes on
a 64-bit machine). The million elements live out in the heap buffer; the vector is a
small handle that owns it.

That layout gives the two operations wildly different prices:

- **Copying** a vector means allocating a new buffer and duplicating *n* elements.
  O(n) — the 150 microseconds from step 1.
- **Moving** a vector means copying those three words into the destination and
  blanking them in the source. The destination now owns the old buffer; the source
  owns nothing. O(1), whether the buffer holds ten bytes or ten gigabytes. No pixel
  is touched.

Moving is obviously cheaper. When is it *legal*? Here is the insight the whole
feature is built on: **copying from a temporary is wasted work.** The temporary is
about to be destroyed — nobody can ever look at it again. So if initializing `shout`
in step 2 were to *steal* the temporary's internal buffer instead of duplicating
it... who could possibly tell the difference?

Nobody. Stealing from a dying object is undetectable, and undetectable means allowed.
C++ therefore gives every type the option of a second, cheaper handoff — and routes
to it automatically whenever the source is an rvalue.

### 4. `std::move` is a label, not an action

Rvalues get moved from automatically. But sometimes the dying object has a *name*:
you have a `frame` variable, you are done with it, and you want the cheap handoff
anyway. `std::move` is how you say so. Watch it, with the buffer address as witness:

```cpp
std::vector<int> a(1'000'000, 7);
const int* before = a.data();        // address of a's heap buffer
std::vector<int> b = std::move(a);   // MOVE construction: b steals the buffer

// b.data() == before   <- verified: the SAME heap buffer, zero elements copied
// a.size() == 0        <- verified: a is now valid but EMPTY
// b.size() == 1000000
```

Unpack the two new things on that middle line.

**`std::move` moves nothing.** It is a *cast* — a compile-time re-labeling that
produces no machine instructions. `std::move(a)` means "treat `a` as an rvalue": *I,
the programmer, promise I no longer need `a`'s contents — you may steal from it.* You
met this exact sentence in lesson 02 with `unique_ptr`; it is true for every type.
And if nothing ends up stealing from the labeled object, nothing happens at all
(verified: `std::move(a);` as a bare statement leaves `a` untouched).

The closest Python gesture is rebinding, then dropping the old name:

```python
b = a      # one object, two names — no copy (Python's default)
del a      # ...now one name again: a clean handoff, nothing duplicated
```

`std::move` is that handoff as one expression — with two C++ twists. First, the
promise is enforced only by convention: nothing stops you touching `a` afterwards,
so the `del` happens in your head (and in code review). Second, C++ *cannot*
actually delete the name: `a` exists until its closing brace, and its destructor
will run there no matter what. Which is exactly why the next two paragraphs exist.

**The stealing is done by the move constructor.** Alongside the copy constructor from
step 1, `vector` provides a **move constructor** — a constructor overload taking
`vector<int>&&`. The `&&` type is an **rvalue reference**: a reference that only
binds to rvalues — genuine temporaries, or lvalues you have labeled with `std::move`.
Ordinary overload resolution then does the routing: pass an lvalue, the copy
constructor runs; pass an rvalue, the move constructor runs. Same call syntax at
every site, radically different cost.

**What is left behind?** Since the source object lives on and will still be
destroyed, a move must leave it in a state that is safe to destroy. A moved-from
standard-library object is **valid but unspecified**: a real, un-corrupted object —
safe to destroy, safe to assign a new value into — but you must not assume anything
about its contents. In practice a moved-from `vector` or `string` is empty, and
`vector`'s *move constructor* guarantees the source ends up empty (verified above).
For classes *you* write, you decide the moved-from state: in the drill, a moved-from
`FrameBuffer` is defined to be a valid 0×0 frame with no pixels, and the asserts
check it.

One calibration note so you don't over-apply the trick: moving only pays when the
object owns heap memory that can be handed over. `std::move` on an `int`, a `double`,
or a small struct of numbers just copies it — there are no guts to steal (verified:
after `int y = std::move(x);` both `x` and `y` hold the value). Fun interview nugget:
a short `std::string` like `"hi"` stores its characters *inline* inside the string
object — the "small string optimization" — so "moving" it copies those bytes too.

### 5. Where you actually write `std::move`: three everyday sites

Step 4 gave you a label and a promise. The natural next question: where in real code
do you apply it? One rule covers every legitimate site:

**You write `std::move` when *you* are done with a variable before the *compiler* can
know it.** A variable with a name and more scope ahead of it looks alive to the
compiler, so the compiler protects its contents. Only you know its useful life is
already over. The label is how you say so. Three places account for nearly all real
uses.

**Site 1 — moving into a container.** You build an object, then store it:

```cpp
std::vector<FrameBuffer> frames;
for (int i = 0; i < n; ++i) {
    FrameBuffer frame = capture_next();     // build this iteration's frame
    frames.push_back(std::move(frame));     // store it WITHOUT copying the pixels
}                                           // the emptied frame dies here, cheaply
```

Without the label, `push_back(frame)` must copy every pixel — `frame`'s scope has
another line to go, so as far as the compiler knows, you still need it. With the
label, the vector steals the buffer. The drills prove it with the counters:
copies 0, moves 1.

**Site 2 — handing to a sink.** A **sink** is a function whose parameter is by value
because the function intends to *keep* the object:

```cpp
void enqueue(FrameBuffer fb);           // by-value parameter: "give me the frame"

FrameBuffer frame = capture_next();
enqueue(std::move(frame));              // hand it over; frame is empty afterwards
// frame.width() ...                    <- WRONG: you promised you were done with it
```

The drill `consume` is exactly this shape.

**Site 3 — into a member, inside a constructor.** The standard spelling for "accept
a value and keep it":

```cpp
class Robot {
    std::string name_;
public:
    Robot(std::string name) : name_(std::move(name)) {}   // take by value, move into place
};
```

The parameter `name` is already the constructor's own copy (lesson 01, step 4), so
moving it into the member finishes the job without a second copy. This compiles and
behaves as shown (verified).

Now notice what all three sites share: **after the handoff, the variable keeps
existing.** The loop's `frame` has a line of scope left. `name` lives until the
constructor's closing brace. That is exactly why you had to speak up — without the
label, the compiler protects contents you no longer want.

Hold that thought, because there is one place where the variable does *not* keep
existing afterwards — and there the label stops helping and starts hurting. That
mistake gets step 7 to itself, right after we settle who writes all this stealing
machinery in the first place.

### 6. The Rule of Three, the Rule of Five, the Rule of Zero

Now flip from *using* moves to *providing* them. `vector` ships with a correct copy
constructor and move constructor. The classes you write need to get theirs from
somewhere too.

The compiler auto-generates six member functions if you don't write them; the five
that matter here are called the **special member functions** (the sixth is the
default constructor). For a class whose members all clean up after themselves —
vectors, strings, smart pointers — the generated ones are exactly right: generated
copy copies each member, generated move moves each member, generated destructor
destroys each member.

The trouble starts when a class owns a resource *directly* — a raw `new[]` pointer, a
file descriptor, a GPU buffer handle. The generated copy constructor copies members,
and copying a raw pointer member is a **shallow copy**: two objects now hold the same
address, both destructors free it, crash (lesson 02's double-free). Fixing that means
writing the copy operations yourself — hence the classic **Rule of Three**: *if a
class needs a hand-written destructor, copy constructor, or copy assignment operator,
it almost certainly needs all three.* They exist for the same reason (the resource),
so handling it in one place but not the others is a latent bug.

C++11 added moves, extending it to the **Rule of Five** — the full set:

```cpp
~T();                        // 1. destructor        release the resource
T(const T& other);           // 2. copy constructor   duplicate it (deep copy)
T& operator=(const T&);      // 3. copy assignment    duplicate into existing object
T(T&& other) noexcept;       // 4. move constructor   steal it
T& operator=(T&&) noexcept;  // 5. move assignment    steal into existing object
```

And here is which one a given line invokes — worth memorizing cold, interviewers ask
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

**The `noexcept` clause.** `noexcept` on a function is a promise that it will not
throw an exception; if the promise is broken, the program terminates instead of
continuing. Move constructors must make this promise, for a concrete reason: when a
`vector<T>` grows, it relocates its existing elements into the new, bigger buffer —
and it will only *move* them if `T`'s move constructor is `noexcept`. Otherwise it
*copies* every element, so that it can roll back intact if an exception strikes
mid-relocation. Measured with counters, relocating 2 elements while pushing a 3rd:

```cpp
// move ctor noexcept:      1 copy (the new element) + 2 moves   <- verified
// move ctor NOT noexcept:  3 copies, 0 moves                    <- verified
```

Forget one keyword, and every reallocation of every `vector<FrameBuffer>` in the
program degrades to deep copies. Silently.

**The Rule of Zero.** The modern punchline, previewed in lesson 02 — and it should
feel familiar, because it is where Python classes live by default. Across the whole
Python drill set you wrote a dozen classes and never once wrote `__del__` or a copy
hook: fields clean up after themselves, and sensible behavior comes generated for
free (the same energy as `@dataclass` writing your `__init__` and `__eq__` in Python
set 06). Rule of Zero is C++ arriving at the same place: since the generated members
do the right thing when every field is self-cleaning, *hold resources through
`vector` / `string` / `unique_ptr` members and write none of the five.* Rule-of-Zero
classes make up ~95% of real code; hand-written Rule-of-Five classes are reserved
for the low-level 5% that wrap a raw resource (or need observable copy/move
behavior — which is precisely why the drill's instrumented `FrameBuffer` writes them
out).

### 7. Returning by value is free — don't "help"

After step 5's three sites, one more habit suggests itself — and it is exactly
wrong, so it gets its own step. If copying
is expensive and `std::move` prevents copies, surely returning a big object should be
`return std::move(result);`?

No — and it is the opposite of harmless. To see why, first be clear about why
returning needs any machinery at all.

A function's local variables live in the function's private workspace, and that
workspace is torn down the moment the function returns (lesson 02's stack — and the
same fact you met in Python's closure lesson: locals stop existing when the function
ends). So a returned local cannot simply *continue existing* outside the function:
its box is about to vanish, and the caller's variable is a different box in a
workspace that survives. The value must get from the dying box into the surviving one
before the teardown. The only real question is what that trip costs.

Python never made you ask this, because in Python every object lives on the heap and
`return result` hands back a *reference* — teardown kills names, never objects. In
C++ the local object itself is in the doomed workspace, so C++ must answer. And its
answer is better than you'd guess — the trip is usually free:

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

"Constructed directly in the caller's variable" sounds like compiler folklore, so
watch it happen — print the object's own address on both sides of the return:

```cpp
std::vector<int> make() {
    std::vector<int> v = {1, 2, 3};
    std::printf("inside:  %p\n", static_cast<void*>(&v));
    return v;                          // plain return — elision-eligible
}
auto a = make();
std::printf("caller:  %p\n", static_cast<void*>(&a));
// inside:  0x16b742bc0
// caller:  0x16b742bc0    <- the SAME address (verified). There was only ever ONE
//                            object: the function was filling in the caller's
//                            variable from its first line.
```

Swap the return for `return std::move(v);` and the two lines print *different*
addresses (verified) — now two objects exist, and a real move runs between them.

Which is why `return std::move(local);` is an anti-pattern with a name —
**pessimizing move**: wrapping the local in `std::move` changes the returned
expression's type in a way that *disqualifies* it from elision, so you pay a move
where you could have paid nothing (verified: 1 move instead of 0). The compiler will
tell you (verified, from
`clang++ -std=c++20 -Wall -Wextra -Werror=return-type` on exactly this code):

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
clang++ -std=c++20 -Wall -Wextra -Werror=return-type -o /tmp/moves starter.cpp && /tmp/moves
```

Background from earlier lessons: ownership, destructors, and `unique_ptr` in
[`../02_ownership_and_raii/`](../02_ownership_and_raii/README.md).

## The road ahead

The skill you just built — knowing exactly when bytes get duplicated, and handing
buffers over instead — is the daily bread of inference work. On a GPU the stakes go
up an order of magnitude: the expensive copy is no longer RAM-to-RAM at ~150 µs per
frame, it is host-to-device across the PCIe bus, and a pipeline that carelessly
copies tensors at every stage boundary will spend more time shipping bytes than
computing. The wrapper types around GPU buffers in real engines are exactly the
Rule-of-Five class you wrote today: movable, non-trivially-copyable owners of a raw
resource. And the questions you can now answer with counters — "did this line copy?",
"who owns this buffer now?" — become the questions you will answer with a profiler.
Next stop on the cost trail: lesson 05 looks at *why* that 2 MB copy costs what it
costs — memory layout and caches.
