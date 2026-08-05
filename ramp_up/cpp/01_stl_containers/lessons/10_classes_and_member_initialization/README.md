# 10 — Classes and Member Initialization

This lesson caps the module by turning loose data into a class that cannot be
wrong: private state, a public interface, and a constructor that makes the
members agree before anyone can look at them. After it, you can write a
member-initializer list and say why it beats assigning in the body, recite
the initialization-order rule that bites silently, and mark accessors
`const`. The drill is a repair job: fix `CameraFrame`'s constructor so
width, height, and pixel buffer describe the same image from the instant a
frame exists. Every output shown in a comment was run and verified.

## The problem this lesson solves

Lessons 07–09 passed data around as structs and bare vectors — which means
any code anywhere could set `width_` to 4 while the pixel vector holds 6
elements. Nothing ties the facts together. Python taught you classes with
`__init__`, but Python privacy is a naming convention: `_field` is a polite
request, and any caller can still reassign anything. C++ classes make the
compiler the wall. The catch is that translating `__init__` literally —
create the object, then assign fields in the body — is exactly the broken
pattern the starter hands you, and it drags in three traps: wasted work,
members that cannot be assigned at all, and an initialization-order rule
that ignores what you wrote.

## The lesson

### `class`: the compiler-enforced version of `_private`

A **class** is a struct whose members are **private** by default — readable
and writable only from the class's own member functions. The layout
convention: `public:` interface first, `private:` state last, members named
with a trailing underscore (the mirror image of Python's leading underscore
— but enforced):

```cpp
CameraFrame frame(4, 3, 7);
frame.width_ = 999;          // reaching for private state
```

```text
private.cpp:20:11: error: 'width_' is a private member of 'CameraFrame'
```

The payoff is the **invariant** — a fact the class guarantees at all times,
here `pixels_.size() == width_ * height_`. Since no outside code can touch
the three members, the constructor is the only place the invariant can be
made — or broken.

### The constructor and its member-initializer list

A **constructor** is the function that runs when an object is created: it
bears the class's name and has no return type. Python's `__init__` receives
an already-existing `self` and bolts attributes on, one assignment at a
time. A C++ object's members must exist *before* the constructor's body
runs, so C++ gives construction its own slot: the **member-initializer
list** after the `:`, where each member is constructed directly with its
final value:

```cpp
CameraFrame(std::size_t width, std::size_t height, std::uint8_t fill)
    : width_(width),
      height_(height),
      pixels_(width * height, fill) {}   // lesson 07's fill constructor, direct
```

By the time the (empty) body runs, all three members are fully built.
Assigning in the body instead — the literal `__init__` translation — means
"default-construct everything, then overwrite": wasted work for a vector,
and impossible for members that cannot be assigned at all. The compiler
proves the point on a `const` member:

```cpp
class Frame {
public:
    Frame(std::size_t w) { width_ = w; }   // __init__ style: assign in the body
private:
    const std::size_t width_;
};
```

```text
constmember.cpp:5:5: error: constructor for 'Frame' must explicitly initialize the
      const member 'width_'
constmember.cpp:5:35: error: cannot assign to non-static data member 'width_' with
      const-qualified type 'const std::size_t'
```

Habit: a well-written constructor usually has an empty body — the list does
the work.

### Members initialize in declaration order — not list order

The rule that gives this lesson its name: the initializer list's *spelling*
does not control execution order; the order of the member *declarations*
does. It matters the moment one initializer reads another member:

```cpp
struct Frame {
    Frame(std::size_t w, std::size_t h)
        : width_(w), count_(width_ * h) {}   // width_ written first — LOOKS safe
    std::size_t count_;    // declared first  -> initialized first
    std::size_t width_;    // declared second -> initialized second
};
```

`count_` is constructed first and reads `width_` before `width_` has been
initialized — garbage, silently. This compiles; `-Wall` is what saves you:

```text
initorder.cpp:5:11: warning: field 'width_' will be initialized after field 'count_'
      [-Wreorder-ctor]
initorder.cpp:5:29: warning: field 'width_' is uninitialized when used here
      [-Wuninitialized]
```

Two habits make the trap unhittable: write the list in declaration order,
and initialize members from *parameters*, never from other members — the
drill's `pixels_(width * height, fill)` uses the parameters and is immune by
construction.

### `const` member functions

```cpp
std::size_t width() const { return width_; }   // trailing const: a promise
```

The trailing **`const`** promises that the member function does not modify
the object — lesson 04's look-don't-touch, moved inside the class. It is
what makes the call legal on a `const CameraFrame`, and the starter's `main`
constructs exactly that, so every accessor must carry it. The compiler holds
the line from the caller's side too — given a non-`const`
`void clear_pixels() { pixels_.clear(); }`:

```cpp
const CameraFrame frame(4, 3, 7);
frame.clear_pixels();
```

```text
constcall.cpp:21:5: error: 'this' argument to member function 'clear_pixels' has
      type 'const CameraFrame', but function is not marked const
```

Rule: mark every non-mutating member function `const` the moment you write
it.

### The Rule of Zero

What about cleanup? Python leaves the pixel buffer to the garbage collector,
eventually. C++ destruction is deterministic: when `frame` goes out of
scope, its members are destroyed on the spot, and `pixels_` — a vector, an
owner (lesson 07) — frees its own block. Because every member manages its
own resources, `CameraFrame` needs no destructor and no hand-written copy
code. That design guideline has a name — the **Rule of Zero**: build classes
out of self-managing members and write zero cleanup lines.

## Muscle memory

Type these until they come out without thinking:

```cpp
class Frame {
public:
    Frame(std::size_t w, std::uint8_t fill)
        : width_(w), pixels_(w, fill) {}          // list in DECLARATION order, from params
    std::size_t width() const { return width_; }  // const on every non-mutating member
private:
    std::size_t width_;                           // trailing _ marks members
    std::vector<std::uint8_t> pixels_;            // self-managing member -> Rule of Zero
};
```

## The drills

One drill, and for once the code is already written around you — the
starter's constructor is the only broken part.

### `CameraFrame(width, height, fill)`

Repair the member-initializer list so the three arguments actually
initialize `width_`, `height_`, and `pixels_`.

```cpp
const CameraFrame frame(4, 3, 7);
frame.width()          // -> 4
frame.pixel_count()    // -> 12    width * height
frame.checksum()       // -> 84    12 pixels x 7 — lesson 07's checksum, now a method
```

The starter initializes `width_(0), height_(0), pixels_()` and discards all
three arguments — and note that it *compiles cleanly*: a constructor that
establishes the wrong state is a logic bug the type system cannot see, which
is exactly why the asserts exist. Replace the list with `width_(width),
height_(height), pixels_(width * height, fill)` and leave the body empty.
Keep the member declarations and their order untouched (the drill's rule),
keep the list in that same order, and use *parentheses* for the vector:
inside an initializer list, `pixels_{width * height, fill}` hits lesson 07's
brace trap and means "a vector containing those two values".

**Where you'll see it:** every serious C++ type you will meet — message
classes, image and tensor wrappers, node handles — establishes its
invariants in a constructor exactly like this, and "width, height, and a
buffer that must agree" is literally the shape of the image types in vision
and inference libraries. In interviews, "design a class for X" is a stock
question where initializer-list fluency and `const`-correct accessors are
the first two things a reviewer scans for — and the declaration-order rule
is a favorite follow-up probe ("what happens if I reorder the list?").

## How to practice

Write your attempt in `starter.cpp`, then run the tests against it:

```bash
PRACTICE=1 uv run pytest ramp_up/cpp/01_stl_containers/lessons/10_classes_and_member_initialization -q
```

Drop the `PRACTICE=1` to run them against the reference `solution.cpp`. The
untouched starter fails at the `width() == 4` assert first — read which line
the assert names before you fix anything.
