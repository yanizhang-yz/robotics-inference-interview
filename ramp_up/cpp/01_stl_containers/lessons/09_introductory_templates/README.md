# 09 — Introductory Templates

This lesson explains the machinery you have been using since your first
`std::vector<T>`: templates, C++'s way of writing one function or type that
serves many element types without giving up compile-time checking. After it,
you can define a struct template, write a function template the compiler
specializes at each call, and — the daily-work skill — read an instantiation
error back to your own line. The drill, `latest_sample`, serves `double`,
`std::string`, and `int` sample streams from a single definition. Every
output shown in a comment was run and verified.

## The problem this lesson solves

You have typed angle brackets dozens of times now, and someone wrote
`std::vector` exactly once. Without templates, generic code has two bad
substitutes. You can copy-paste a `DoubleSample`, `StringSample`,
`IntSample` — three definitions that drift apart at the first bug fix. Or
you can do what Python does: **duck typing**, where every function is
generic for free because nothing is checked until the line actually runs —
and the wrong-type crash arrives at runtime, possibly on a deployed robot.
Templates refuse both costs: one definition, checked per type at compile
time, with zero runtime overhead. The price of admission is some new syntax
and error messages you must learn to read.

## The lesson

### A struct template is a recipe, not a type

```cpp
template <typename T>
struct TimedSample {
    std::int64_t timestamp_ns;
    T value;
};
```

The **`template <typename T>`** prefix declares that what follows is a
**template**: a compile-time recipe with a placeholder type `T`.
`TimedSample` by itself is not a type — you cannot declare a variable of it.
Naming `TimedSample<double>` **instantiates** the recipe: the compiler stamps
out a real struct whose `value` is a `double`.

```cpp
TimedSample<double> joint{10, 0.15};         // a timestamped joint angle
TimedSample<std::string> mode{15, "idle"};   // a timestamped mode string
joint.value    // -> 0.15
mode.value     // -> idle
```

The two instantiations are genuinely different types — different sizes, even
(16 vs 32 bytes on this machine), and no conversion between them:

```cpp
joint = mode;
```

```text
crosstype.cpp:13:11: error: no viable overloaded '='
```

And this is the syntax you already know: `std::vector` is a struct template;
`std::vector<std::uint8_t>` is one instantiation of it.

### Function templates: the compiler fills in `T`

```cpp
template <typename T>
T twice(T x) { return x + x; }

twice(3)                    // -> 6       T deduced as int
twice(1.25)                 // -> 2.5     T deduced as double
twice(std::string("ab"))    // -> abab    T = std::string; + concatenates
```

**Deduction** is why the call sites carry no angle brackets: the compiler
infers `T` from the argument's type. Each distinct `T` gets its own compiled,
fully type-checked copy of the function — as fast as if you had written
`twice_int` and `twice_double` by hand, with no runtime type tag deciding
anything.

### Duck typing, moved to compile time

The Python `twice` is `def twice(x): return x + x` — generic by default, and
checked never, until execution reaches it with a bad argument and raises
`TypeError` mid-flight. A template is the same genericity with the check
moved to compile time: the body may use any operation it likes, and the
requirement is enforced the moment you instantiate with a type that cannot
deliver:

```cpp
twice(std::vector<int>{1, 2});   // vectors have no + operator
```

```text
ducktype.cpp:4:25: error: invalid operands to binary expression
      ('std::vector<int>' and 'std::vector<int>')
ducktype.cpp:7:5: note: in instantiation of function template specialization
      'twice<std::vector<int>>' requested here
```

Memorize this two-part shape — every template error has it. The `error:`
points into the template's *body* (sometimes a library header, as in lesson
08's const-sort error), and a `note: ... requested here` chain walks back to
*your* call. Find the "requested here" naming your file; that is where the
fix goes.

### One rule about where templates live

The compiler can only stamp out an instantiation while the full recipe is in
view, so template definitions go in headers — or in the same `.cpp` that
uses them — rather than being split declaration-in-header,
definition-in-one-`.cpp` like lesson 02 taught for ordinary functions. Break
the rule — declare `twice` in one file, define it in another — and the
failure is not a compile error but lesson 01's *other* kind: the calling
file compiles fine against the declaration, no instantiation ever gets
generated, and the linker reports the missing symbol:

```text
Undefined symbols for architecture arm64:
  "int twice<int>(int)", referenced from:
      _main in split_main-9464c5.o
```

The starter keeps everything in one file, so this costs you nothing today;
recall it the first time a header template "mysteriously" carries its whole
body.

### Two vector tools the drill needs

**`empty()`** is true when `size()` is 0 — the counterpart of Python's
`if not lst:`. **`back()`** is the last element — Python's `lst[-1]` — and
calling it on an empty vector is undefined behavior, not an exception, which
is why the emptiness guard always comes first. `&samples.back()` takes the
last element's address: lesson 05's borrow.

## How interviewers test this

- **Prediction probe:** Identify the concrete types instantiated for each call to `latest_sample`.
- **Implementation probe:** Find the latest typed sample.
- **Follow-up probe:** Explain compile-time reuse and how template errors expand at an instantiation site.

## Muscle memory

Type these until they come out without thinking:

```cpp
template <typename T>                        // recipe prefix: the next thing is generic
struct Sample { std::int64_t t; T value; };  // Sample<double>, Sample<int>: distinct types
template <typename T>
const T* pick(const std::vector<T>& v);      // T deduced from the argument at each call
latest_sample(joints)                        // no angle brackets — deduction fills them in
if (v.empty()) return nullptr;               // guard BEFORE back()
return &v.back();                            // borrow the newest element
// template error: error -> body; note "requested here" -> YOUR call site
```

## The drills

One function in `starter.cpp` — but it must compile for three different `T`s
from a single definition. Keep the `TimedSample` struct exactly as written.

### `latest_sample(samples)`

Return a pointer to the newest sample — the last one, since this stream
stores samples in arrival order — or `nullptr` when there are none.

```cpp
const std::vector<TimedSample<double>> joints{{10, 0.1}, {20, 0.2}};
latest_sample(joints)->value                     // -> 0.2     T deduced as double
latest_sample(modes)->value                      // -> idle    same code, T = std::string
latest_sample(std::vector<TimedSample<int>>{})   // -> nullptr empty stream
```

Shape: guard, then borrow — `if (samples.empty()) return nullptr;` followed
by `return &samples.back();`. This is lesson 05's nullable-pointer contract
inside a template: the return type `const TimedSample<T>*` says "may be
null, check before `->`" and owns nothing — the vector keeps the sample, so
the pointer stays valid only while the vector is alive and un-grown (lesson
07's invalidation rule). The traps the tests check: the empty guard must come
first, because `back()` on an empty vector is undefined behavior rather than
a catchable error; and one definition must serve `double`, `std::string`,
and `int` vectors — if you find yourself writing three overloads, re-read
the lesson.

**Where you'll see it:** "give me the newest reading" is the most common
query in a robot — latest joint state for the controller, latest IMU sample
for the estimator, latest camera frame for inference — and timestamped
generic wrappers exactly like `TimedSample` are how logging and message
layers stamp arbitrary payloads. In interviews, templates are the machinery
under every container question ("implement a generic pair/stack" is a stock
warm-up), and calmly walking an instantiation error back to the offending
call reads as real C++ mileage.

## How to practice

Write your attempt in `starter.cpp`, then run the tests against it:

```bash
PRACTICE=1 uv run pytest ramp_up/cpp/01_stl_containers/lessons/09_introductory_templates -q
```

Drop the `PRACTICE=1` to run them against the reference `solution.cpp`.
