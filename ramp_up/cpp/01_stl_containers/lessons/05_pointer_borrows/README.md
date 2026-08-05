# 05 — Pointer Borrows

This lesson introduces your first pointer, in the one narrow role modern
C++ still uses a raw pointer for: a *nullable, non-owning* handle — "here
is someone else's object, or nothing". After it, you can take an address
with `&`, read through a pointer with `*` and `->`, guard with `nullptr`,
and write the drill: a registry lookup that returns a borrowed view of a
matching sensor, or `nullptr` for "not found". Every output shown in a
comment was run and verified.

## The problem this lesson solves

Lesson 04's references have a hole: a reference must bind to a real object
at birth and can never be "nothing". So how do you write `find` — a
function whose honest answer is sometimes *there is no such element*? In
Python you return the object or `None`. A C++ reference cannot say `None`;
returning a copy can't say it either (which `Sensor` would you return?),
and inventing a sentinel `Sensor` is a lie waiting to be read. The
language's answer is the pointer: an alias, like a reference, but one that
can be null — at the price that *you* must check it before every use, and
that it owns nothing: the pointed-at object's lifetime is somebody else's
business.

## The lesson

### `&x` — the other meaning of `&`

Lesson 04 promised this: in an *expression*, `&` takes an **address**.
Every object lives somewhere in memory; `&x` produces that location. A
**pointer** is a variable that holds an address, and its type spells what
it points at — `double*` reads "pointer to double":

```cpp
double x = 5.0;
double* p = &x;   // p holds x's address — "p points at x"
```

Nothing was copied and no second `double` exists — `p` is a note saying
where `x` lives.

### `*p` — follow the pointer

In an expression, `*` **dereferences**: it follows the address to the
object itself. Reads and writes through `*p` are reads and writes of `x`:

```cpp
*p        // -> 5.0    read x through the pointer
*p = 7.0; // write through the pointer...
x         // -> 7.0    ...and it lands in x — one object, two routes to it
```

Like `&`, the symbol `*` flips meaning by position: in a *declaration*
(`double* p`) it builds a pointer type; in an *expression* (`*p`) it
dereferences. So far this is lesson 04's alias with extra spelling — the
differences come next.

### `->` — member access through a pointer

For a struct, `(*q).name` works but is a chore; `q->name` is the same
thing:

```cpp
Sensor s{"wrist-camera", 30.0};
Sensor* q = &s;
q->rate_hz      // -> 30.0    shorthand for (*q).rate_hz
```

The arrow is the visual tell of pointer code — when you see `->`, someone
is reaching through an address.

### `nullptr` — a pointer's None

Here is what a reference cannot do. **`nullptr`** is the pointer value
that points at nothing — C++'s `None`, but only pointers can hold it:

```cpp
const Sensor* none = nullptr;
none == nullptr    // -> true
if (none) { ... }  // body skipped — a null pointer is falsy, like None
```

A pointer in a boolean context converts exactly like Python's
`if p is not None:` idiom — `if (p)` means "p points at something". That
makes `T*` a self-documenting return type: *may be absent, check me*.

The trap is Python-shaped but harsher. Touch an attribute on `None` and
Python throws `AttributeError` with a traceback naming the line. Follow a
null pointer in C++ and you get **undefined behavior** — no guaranteed
diagnostic at all. In the verified run below, the process simply dies:

```cpp
Sensor* camera = nullptr;
std::cout << camera->rate_hz;   // ran: killed by a segmentation fault —
                                // exit code 139, no message, no line number
```

Hence the iron habit: **check before the first dereference**, once, at the
boundary where "maybe" must become "definitely".

### A borrow, not a box

The second half of the contract: this pointer **owns nothing**. Nothing is
created when you take `&sensor`, nothing must be freed when the pointer
goes away — it is a *borrowed view* of an object that belongs to someone
else (here, to the vector). Two consequences:

`const Sensor*` means look, don't touch — lesson 04's `const`, applied
through a pointer, and enforced the same way:

```cpp
const Sensor* camera = &s;
camera->rate_hz = 60.0;   // write through a read-only borrow
```

```text
constptr.cpp:8:21: error: cannot assign to variable 'camera' with
    const-qualified type 'const Sensor *'
```

And the borrow is only valid while the owner keeps the object alive *and
in place*. Destroy the vector and the pointer **dangles** — it holds the
address of a corpse. Even growing the vector (`push_back`) can move every
element to new storage, dangling all old pointers. Using a dangling
pointer is undefined behavior, so there is no honest output to show you —
the rule is simply: do not keep a borrowed pointer across changes to the
container; look it up again.

### Returning a pointer into a vector

The pieces assemble into the drill's shape. A `const Sensor&` loop
variable (lesson 04) aliases each element *in place*, so `&sensor` is the
address of the element inside the vector — not of any copy:

```cpp
const Sensor* camera = find_sensor(sensors, "wrist-camera");
camera->rate_hz         // -> 30.0
camera == &sensors[0]   // -> true   the vector's own element, borrowed
```

## Muscle memory

Type these until they come out without thinking:

```cpp
double* p = &x;             // & in an expression: address-of
*p                          // * in an expression: follow the pointer
p->member                   // (*p).member
if (p != nullptr) { ... }   // guard before the first dereference
return &sensor;             // borrow an element someone else owns
return nullptr;             // "not found" — the honest absent answer
const Sensor* s             // read-only borrow: look, don't touch
```

## The drills

One function in `starter.cpp`; `main` checks the found case, a member read
through the pointer, and the not-found case.

### `find_sensor(sensors, name)`

Return a read-only borrow of the first sensor whose name matches, or
`nullptr` if none does.

```cpp
const std::vector<Sensor> sensors{
    {"wrist-camera", 30.0},
    {"joint-encoder", 100.0},
};
find_sensor(sensors, "wrist-camera")->rate_hz     // -> 30.0
find_sensor(sensors, "missing") == nullptr        // -> true
```

The shape is a lesson-04 loop with two returns:

```cpp
for (const Sensor& sensor : sensors) {
    if (sensor.name == name) {
        return &sensor;     // address of the element inside the vector
    }
}
return nullptr;             // loop ended: honestly absent
```

The trap the tests will surface: write the loop as
`for (const Sensor sensor : sensors)` — no `&` — and `sensor` is a
loop-local *copy* (lesson 03), so `&sensor` borrows an object that dies
every iteration. The compiler sees it coming:

```text
warning: address of stack memory associated with local variable 'sensor'
    returned [-Wreturn-stack-address]
warning: loop variable 'sensor' creates a copy from type 'const Sensor'
    [-Wrange-loop-construct]
```

Warnings, not errors — the program still builds and may even appear to
pass. Treat `-Wreturn-stack-address` as a stop-everything alarm: it is the
dangling borrow from the lesson, caught at birth.

**Where you'll see it:** interview data structures run on this exact
contract — linked lists and trees are structs whose `next`, `left`, and
`right` are nullable pointers, and every list or tree problem is
null-check discipline in a loop ("Reverse Linked List", "Merge Two Sorted
Lists", any traversal). Lookup-that-may-miss is also the everyday shape:
a robot's sensor registry queried for optional hardware — `nullptr` for a
robot without that camera lets the caller pick a fallback instead of
crashing; an inference server checking a cache for a compiled model or a
loaded checkpoint before paying the load cost. And `const T*` versus `T*`
in an API tells you at a glance, like lesson 04's references, whether the
borrower may write.

## How to practice

Write your attempt in `starter.cpp`, then run the tests against it:

```bash
PRACTICE=1 uv run pytest ramp_up/cpp/01_stl_containers/lessons/05_pointer_borrows -q
```

Drop the `PRACTICE=1` to run them against the reference `solution.cpp`.
The untouched starter returns `nullptr` for everything, so it fails at
`assert(camera != nullptr)` — note that `main` checks the pointer *before*
`camera->rate_hz`, the exact guard-then-dereference order the lesson
preaches.
