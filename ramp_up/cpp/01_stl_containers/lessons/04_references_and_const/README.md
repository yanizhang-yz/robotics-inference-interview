# 04 — References and Const

This lesson gives you the parameter-passing decision you will make in every
C++ function you ever write: plain `T` (a copy), `const T&` (look, don't
touch), or `T&` (edit the caller's object). After it, you can pick the right
one on sight, explain the choice in a sentence, and read any C++ signature as
a statement of intent. The drill applies both reference forms to a robot
joint state: inspect it without copying, then clamp it in place. Every output
shown in a comment was run and verified.

## The problem this lesson solves

Lesson 03 showed that passing by value copies — safe, but wasteful for a big
vector, and useless when the function's whole *job* is to modify the caller's
object. The instinct from garbage-collected languages ("passing an object
shares it automatically") does not transfer: in C++ *you* choose, at the
signature, copy or share, and read-only or writable. Choose wrong and you get
either a silent no-op (mutating your private copy) or an accidental overwrite
of someone else's data. The fix is a three-way convention so standard that
seasoned reviewers read parameter lists the way you read function names.

## The lesson

### A reference is another name, not another object

A **reference** (`T&`) is an alias: a second name bound to an existing
object. It is not a container, it is never null, and no copy happens — after
the binding, the two names *are* the same object. The object being aliased is
called the **referent**:

```cpp
double x = 5.0;
double& alias = x;   // & in a DECLARATION means: alias is another name for x
alias = 7.0;         // writes through the alias...
x                    // -> 7.0   ...land in x. There is only one double here.
```

The `&` reads differently by position, which trips everyone once: in a
*declaration* (`double& alias`) it makes a reference; later, in expression
position (`&x`, lesson 05), it takes an address. Same character, two
meanings.

### The three parameter forms, side by side

Wrap the same operation in the three signatures and watch the caller's
vector:

```cpp
void by_value(std::vector<double> v)  { v.push_back(9.0); }  // copies
void by_ref  (std::vector<double>& v) { v.push_back(9.0); }  // aliases

std::vector<double> d{1.0};
by_value(d);   d.size()   // -> 1   the push_back hit a private copy, now destroyed
by_ref(d);     d.size()   // -> 2   the push_back hit d itself
```

`by_value`'s edit vanishing is the lesson-03 copy boundary; `by_ref`'s edit
landing is the alias at work. The third form is the compromise you will use
most:

### `const T&`: look, don't touch

**`const`** attached to a type means "read-only through this name". A
`const T&` parameter therefore aliases the caller's object — no copy, however
large the object — while the *compiler* enforces that the function only
reads. Any mutation through it is rejected at compile time:

```cpp
void inspect(const std::vector<double>& v) { v.push_back(1.0); }
```

```text
constref.cpp:3:48: error: no matching member function for call to 'push_back'
```

(The wording is odd but the meaning is exact: on a `const` vector, the
mutating members are simply not callable.) This is why `const T&` is the
default for anything bigger than a few doubles: the caller gets a guarantee,
the callee gets free access, nobody pays for a copy.

### The convention, in one table

| Signature | Cost | Callee may modify caller's object? | Says to the reader |
|---|---|---|---|
| `T v` | one copy | no — edits hit the copy | "I need my own scratch value" |
| `const T& v` | free | no — compiler-enforced | "I only read this" |
| `T& v` | free | **yes** | "modifying this IS my job" |

Two habits complete the picture. Reserve `T&` for functions whose *purpose*
is the mutation, and let the name say so: `clamp_in_place(state)` announces
itself; a `T&` hiding in a function called `compute_report` is a trap for the
caller. And keep plain `T` for small things (a `double`, an ID) or when you
genuinely want a private copy to scribble on — for an `int`, a copy is
cheaper than an alias anyway.

### Reading a real signature pair

The drill's two functions are the convention in miniature:

```cpp
double max_abs_position(const JointState& state);        // report: reads only
void   clamp_in_place(JointState& state, double limit);  // mutator: says so
```

No comments needed — the parameter forms *are* the documentation. That is
the payoff of the convention: in review, in an interview, in six months, the
signature alone answers "can this call change my data?"

## Muscle memory

Type these until they come out without thinking:

```cpp
const std::vector<double>& v    // default for any parameter bigger than a scalar
double& alias = x;              // declaration & : alias, not address
for (double p : v)              // copies each element — fine for doubles
void f(T& out)                  // mutation is f's stated job, name it accordingly
std::clamp(x, -limit, limit)    // lesson 01's clamp — reused through a T& below
```

## The drills

Work through `starter.cpp` top to bottom: one reader, one mutator, over

```cpp
struct JointState { std::vector<double> positions; };
```

### `max_abs_position(const JointState& state)`

Return the largest *absolute* position in `state.positions`.

```cpp
JointState state{{-2.0, 0.5, 3.0}};
max_abs_position(state)    // -> 3.0     (|-2.0| = 2.0, |3.0| = 3.0 wins)
```

Walk the vector with a range-for, track the max of `std::abs(p)` (from
`<cmath>`, already included). The `const&` gives you every element for free
and promises the caller nothing moves; start your running max at `0.0` and
this also behaves on an empty vector.

**Where you'll see it:** the safety check before the safety fix — "how far
is the worst joint from home?" Watchdog code does this over joint positions,
velocities, and torques every control cycle. It is also the max-of-transformed
-values pattern ("Maximum Absolute Value" warmups, peak-finding in a signal),
and the shape of every reduction you will later write with
`std::max_element` (lesson 08).

### `clamp_in_place(JointState& state, double limit)`

Pull every position into `[-limit, limit]`, modifying the caller's state.

```cpp
JointState state{{-2.0, 0.5, 3.0}};
clamp_in_place(state, 1.0);
state.positions               // -> {-1.0, 0.5, 1.0}   the ORIGINAL changed
```

Two deliberate contrasts with the reader above: the parameter is `T&`
because mutation is the job, and the loop must be `for (double& p :
state.positions)` — with the `&`. Write `for (double p : ...)` and you clamp
a copy of each element, the vector never changes, and the assertion fails: the
lesson-03 trap, one level down. Reuse `std::clamp(p, -limit, limit)` from
lesson 01.

**Where you'll see it:** this pair — measure, then saturate in place — is the
command-conditioning step of real controllers: clamp the planned joint
targets into the envelope, in place, right before they are sent. The
`for (T& x : xs)` mutate-in-place loop is also the standard shape for
normalizing a feature vector, applying gains, or zeroing a buffer — and a
favorite interview probe: "why didn't your loop change the array?"

## How to practice

Write your attempts in `starter.cpp`, then run the tests against them:

```bash
PRACTICE=1 uv run pytest ramp_up/cpp/01_stl_containers/lessons/04_references_and_const -q
```

Drop the `PRACTICE=1` to run them against the reference `solution.cpp`. The
untouched starter fails at the `max_abs_position` assertion first — a good
chance to practice reading which line the assert names.
