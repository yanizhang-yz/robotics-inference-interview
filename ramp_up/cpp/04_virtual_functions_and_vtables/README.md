# 04 — Virtual Functions and Vtables: inheritance, `override`, and the virtual destructor

After this lesson you will be able to: predict exactly which function body runs for
any call through a pointer, a reference, or a plain value; write a C++ interface
(abstract base class) the way production code does it; explain what a vtable is in
one breath; and nail the two classic interview traps — the non-virtual destructor and
object slicing — that compile cleanly and misbehave at runtime. Every term is defined
the first time it appears; every snippet's output was verified with
`clang++ -std=c++17 -Wall`.

## The problem: one list, many sensor types

Your robot has a camera polling at 30 Hz and a lidar at 10 Hz. Next month it grows an
IMU, then a depth camera. The main loop you *want* to write looks like this:

```cpp
for (const auto& s : sensors) {
    log(s->name(), s->read());   // one loop; it neither knows nor cares which is which
}
```

One list, one loop, and adding a sensor type never touches the loop. For that to
work, three things must be true. One list has to be able to hold different types.
The call `s->read()` has to run the *right* `read` for whatever `s` really is. And
when a sensor is destroyed through that list, the right cleanup has to run.

C++ makes each of those an explicit decision, because of a ground rule the whole
language is built on: **the compiler decides everything it can at compile time.**
Runtime machinery — anything that costs cycles or bytes while the program runs — is
never added silently; you must ask for it. (You'll hear this called the
**zero-overhead principle**: you don't pay for what you don't use.) This lesson is
the story of asking correctly — and of the three quiet bugs that appear when you
forget.

## The lesson

### 1. A base-class pointer can point at a derived object

First, inheritance syntax. `struct Robot : Machine` declares that every `Robot`
contains a complete `Machine` — its fields, its functions — plus whatever `Robot`
adds. We say `Robot` **derives from** the **base class** `Machine`, and that a
`Robot` *is a* `Machine`. (With the `class` keyword you write
`class Robot : public Machine` — members and base classes default to `private` in a
`class` and `public` in a `struct`; that is the only difference between the two
keywords.)

The mechanism is concrete: the compiler lays out each `Robot` object with its
`Machine` part at the front. So inside every `Robot` there literally *is* a
`Machine`, sitting at the same address. Which makes this legal:

```cpp
Robot r;
Machine* p = &r;     // fine: points at the Machine that lives inside r
Machine& ref = r;    // references work the same way
```

This is called an **upcast** — viewing a derived object through a base-class handle.
Why would you want one? It is exactly the opening problem: a
`std::vector<Machine*>` can hold pointers to robots, drills, and conveyors in one
container, and code written against `Machine*` works for all of them. One list, many
types. That much, C++ gives you for free.

The question that matters is what happens when you *call* something through that
pointer.

### 2. The surprise: the pointer's type picks the function

Here is the most natural-looking inheritance code possible — and it does the wrong
thing. Read it, predict the three outputs, then check:

```cpp
#include <iostream>
#include <string>

struct Machine {
    std::string name() const { return "Machine"; }
};
struct Robot : Machine {
    std::string name() const { return "Robot"; }
};

int main() {
    Robot r;
    std::cout << r.name() << "\n";     // -> Robot     fine so far

    Machine* p = &r;                   // the upcast from step 1
    std::cout << p->name() << "\n";    // -> Machine   (!!) the object IS a Robot

    Machine& ref = r;
    std::cout << ref.name() << "\n";   // -> Machine   references: same story
}
```

All three outputs verified. The object never stops being a `Robot` — yet through a
`Machine*`, `Machine::name` runs.

Now apply the ground rule and this stops being surprising. The compiler decides
everything it can at compile time — and it *can* decide this call. It looks at the
**declared type** of the expression: `p` is declared `Machine*`, therefore
`p->name()` is hard-wired to call `Machine::name` before the program ever runs. The
object's real class is never consulted. This is called **static dispatch**: the
function is chosen statically, at compile time, from the type written in the source.
The payoff is speed — a static call compiles to a direct jump, or disappears
entirely when the compiler pastes the function body into the call site. The cost is
what you just saw.

One more name for what happened: `Robot::name` above does not override anything. It
**hides** `Machine::name` — the two are unrelated functions that happen to share a
name, and which one runs depends entirely on the declared type you call through. No
error, no exception. Just the wrong answer, quietly.

### 3. `virtual` opts in to runtime dispatch

What we want for sensors is the opposite rule: *decide at runtime, from the object's
real type.* That is called **dynamic dispatch**, and one keyword on the base-class
function switches it on:

```cpp
struct Machine {
    virtual std::string name() const { return "Machine"; }   // opt in HERE
};
struct Robot : Machine {
    std::string name() const override { return "Robot"; }    // override: next step
};

Robot r;
Machine* p = &r;
std::cout << p->name() << "\n";    // -> Robot    verified: the OBJECT's type decides
Machine& ref = r;
std::cout << ref.name() << "\n";   // -> Robot
```

`virtual` on `Machine::name` says: calls to `name` through a `Machine*` or
`Machine&` are resolved at runtime from the object's real class. Virtual-ness is
inherited — once a function is virtual in the base, every derived function with the
same signature is automatically virtual too, all the way down, whether or not the
derived class repeats the keyword.

#### The mechanism: the vtable

How can a call possibly be resolved at runtime? Interviewers ask for this machinery
by name, and it is worth knowing for real. A **vtable** (virtual table) is a hidden
per-*class* table of function addresses — one slot per virtual function, filled with
the most-derived version for that class. `Machine`'s table has `Machine::name` in
the slot; `Robot`'s table has `Robot::name`. Every *object* of a class with at least
one virtual function carries one hidden pointer — the **vptr** — to its class's
table, planted by the constructor. A virtual call compiles to: follow the object's
vptr, index the slot, call whatever address is there. The pointer you called through
no longer matters; the object brought its own dispatch table.

The vptr is real, and you can see it — objects get bigger:

```cpp
struct Plain    { int x; std::string tag() const { return "plain"; } };
struct WithVptr { int x; virtual std::string tag() const { return "virt"; } };

sizeof(Plain)      // -> 4   just the int
sizeof(WithVptr)   // -> 16  int + hidden 8-byte vptr + alignment padding
                   //        (verified with clang on a 64-bit machine)
```

#### Why C++ makes this opt-in

Now the design question: why isn't every function virtual? Count what dynamic
dispatch costs. Every object grows by a pointer — for a class of one `int`, that was
4 bytes becoming 16, a 4× size increase, which matters enormously when you have a
million small objects marching through a cache. Every call becomes two memory loads
plus an indirect jump. The loads are nearly free; the real price is that a target
unknown until runtime **blocks inlining** — the compiler cannot paste the function
body into the call site, so it also cannot constant-fold, vectorize, or otherwise
optimize across the call boundary. That lost optimization is often 10× the cost of
the indirection itself.

The zero-overhead principle then dictates the answer: C++ refuses to make everyone
pay for what only some need. Functions dispatch statically — free — until you write
`virtual` on the ones where you genuinely need runtime flexibility. You pay exactly
where you chose to.

One rule to carry out of this step: dynamic dispatch needs **both** halves —
`virtual` on the function, *and* a pointer or reference at the call site. Keep that
second half in mind; step 7 shows what happens when you lose it.

### 4. `override`: the typo-catcher

Step 2 showed that a derived function with a merely *similar* signature silently
hides instead of overriding. That makes overriding fragile: one typo and your
function is never called. Watch a one-character bug:

```cpp
struct Machine {
    virtual std::string name() const { return "Machine"; }
};
struct Robot : Machine {
    std::string name() { return "Robot"; }   // forgot const!
};

Robot r;
Machine* p = &r;
std::cout << p->name() << "\n";   // -> Machine   verified
```

`name() const` and `name()` are *different signatures*, so the derived function is a
brand-new function that hides the base one — the virtual slot still holds
`Machine::name`, and every call through a base pointer quietly runs the base
version. (Clang with `-Wall` does flag this one — verified:
`warning: 'Robot::name' hides overloaded virtual function [-Woverloaded-virtual]` —
but it is a warning, the program still builds and still answers wrong; gcc says
nothing without extra flags.) Same trap for a misspelled name, a `float` parameter
where the base says `double`, and every other near-miss.

The fix is the keyword `override`, written after the parameter list. It tells the
compiler: "I intend this to override a base-class virtual — fail the build if it
doesn't." The near-miss becomes a hard error at the exact line (verified, clang):

```cpp
struct Robot : Machine {
    std::string name() override { return "Robot"; }
    // error: non-virtual member function marked 'override' hides virtual
    //        member function
    // note:  different qualifiers ('const' vs unqualified)
};
```

Rule: **every** overriding function gets `override`, no exceptions — it upgrades
"silently wrong" to "does not compile." Style note: in the derived class write
`override` *instead of* repeating `virtual` (`override` already implies it); the
drills follow that convention.

### 5. The classic: the virtual destructor

You met destructors in lesson 02: a destructor is the function that runs, at a
deterministic line, when an object dies — and in RAII code it is where files close,
locks release, and memory frees. Inheritance adds a sharp question: *when an object
dies through a base-class pointer, which destructor runs?*

Set the stage with the correct, everyday case — a stack object dies at its brace and
tears down completely:

```cpp
#include <iostream>
#include <memory>

struct GpuBuffer {
    ~GpuBuffer() { std::cout << "GPU memory freed\n"; }
};

struct Model {
    ~Model() { std::cout << "~Model\n"; }        // NOT virtual — the bug, wait for it
};
struct TrtModel : Model {                        // a TensorRT-backed model
    std::unique_ptr<GpuBuffer> buf = std::make_unique<GpuBuffer>();
    ~TrtModel() { std::cout << "~TrtModel\n"; }
};

int main() {
    {
        TrtModel m;
    }   // prints: ~TrtModel
        //         GPU memory freed
        //         ~Model
}
```

Note the order (verified): derived destructor body first, then the derived class's
*members*, then the base — the exact reverse of construction. All good. Now the same
class used polymorphically, the way every plugin registry and model zoo uses it:

```cpp
Model* p = new TrtModel;   // or handed to you by a factory
delete p;                  // prints: ~Model
                           // ...and NOTHING else. ~TrtModel never ran.
                           // buf's destructor never ran. GPU memory LEAKED.
```

Why: a destructor is a member function like any other, and this one is not virtual —
so `delete p` static-dispatches on the pointer's type (step 2, one more time) and
runs *only* `Model::~Model`. The derived half of the object is never torn down.
Honesty about the fine print: the C++ standard formally declares this **undefined
behavior** — deleting a derived object through a base pointer whose destructor is
non-virtual means the program is allowed to do anything at all. What clang and gcc
actually do is what you see above (verified): base destructor only, leak included,
no diagnostic.

The fix is one word, in one place — the base:

```cpp
struct Model {
    virtual ~Model() { std::cout << "~Model\n"; }
};

Model* p = new TrtModel;
delete p;                  // prints: ~TrtModel
                           //         GPU memory freed
                           //         ~Model      — full teardown, derived-then-base
```

With the destructor virtual, `delete p` dynamically dispatches to the *object's*
destructor, which then runs the chain in reverse-construction order (verified).

Memorize the rule as an interview sound bite: **any class with virtual functions
gets a virtual destructor** — if code deletes derived objects through base pointers,
it is mandatory. When the base has no cleanup of its own, write
`virtual ~Model() = default;` (that is `= default` from lesson 02: "generate the
usual body"). And the follow-up interviewers love: `std::unique_ptr<Model>` does
**not** save you — at scope end it performs `delete` on a `Model*`, exactly the
broken call above. Smart pointer, same rule.

### 6. Pure virtual functions and abstract classes: a class that is only a promise

Sometimes the base version of a function has no sensible body — what would a generic
`Sensor::read()` even return? C++ lets you declare the slot and refuse to fill it. A
**pure virtual function** is a virtual function marked `= 0`, meaning "no body here;
deriving classes must provide one." A class with at least one pure virtual function
is an **abstract class** — a class that is only a promise — and the compiler refuses
to instantiate it (verified error text):

```cpp
struct Sensor {
    virtual ~Sensor() = default;             // step 5's rule, applied on reflex
    virtual std::string name() const = 0;    // = 0 makes it pure virtual
    virtual double read() = 0;
};

Sensor s;   // error: variable type 'Sensor' is an abstract class
            // note:  unimplemented pure virtual method 'name' in 'Sensor'
```

An abstract class whose every function is pure virtual is what other contexts call
an *interface*: no data, no behavior, just a contract. C++ has no separate keyword
for it — plain inheritance is the single mechanism, and a class may inherit from
several bases at once when it needs to satisfy several contracts (the sharp edges of
multiple inheritance are a story for another lesson).

A derived class must override *every* pure virtual function or it remains abstract
itself. Once `Camera` overrides both `name()` and `read()`, it is **concrete** —
instantiable — and the idiomatic way to use the family combines this lesson with
lesson 02:

```cpp
std::unique_ptr<Sensor> s = std::make_unique<Camera>();   // interface + ownership
s->read();                                                // dynamic dispatch
// scope end: ~Camera runs, then ~Sensor — because ~Sensor is virtual
```

### 7. Object slicing: when the object doesn't fit

Step 3 left a warning hanging: dynamic dispatch needs a pointer or reference at the
call site. Here is what happens without one — and it is stranger than "dispatch
turns off."

Recall lesson 01's rule one final time: a variable *is* its object, and assignment
copies the whole thing. Now combine that with inheritance and ask: copy *into what*?
A `Machine` variable is a box exactly big enough for a `Machine`. So:

```cpp
Robot r;                           // the virtual Machine/Robot from step 3
Machine m = r;                     // compiles fine! copies ONLY the Machine part
std::cout << m.name() << "\n";     // -> Machine   verified... though name() is virtual
```

This is **object slicing**: initializing or assigning a base-class *value* from a
derived object copies just the base sub-object (the front of the box, from step 1)
and throws the derived part away. The `Robot`-ness didn't come along and get
suppressed — it was never copied. `m` is a genuine, complete `Machine`; its vptr
points at `Machine`'s vtable; dynamic dispatch is working perfectly, on the wrong
object. `virtual` cannot help here. There is nothing left to dispatch to.

The version that gets people in interviews (and code review) is the by-value
parameter, because it *looks* polymorphic:

```cpp
std::string describe(Machine m)         { return m.name(); }  // by VALUE: slices
std::string describe2(const Machine& m) { return m.name(); }  // by REFERENCE: dispatches

Robot r;
describe(r);    // -> "Machine"   verified: r was sliced into the parameter
describe2(r);   // -> "Robot"     verified: a reference to the real object
```

Same trap one more way: `std::vector<Machine>` stores `Machine` *values*, so
`v.push_back(r)` slices — `v[0].name()` returns `"Machine"` (verified). A
heterogeneous collection must hold pointers, which is why the drills (and all
production code) use `std::vector<std::unique_ptr<Sensor>>`.

Rule: **polymorphic types travel by pointer or by reference — never by value.** And
a pleasant bonus of step 6: if the base class is abstract, by-value parameters and
`vector<Base>` *don't compile* (you can't instantiate an abstract class), so a
pure-virtual interface turns this whole silent-trap step into loud compile errors.
One more reason real codebases keep their base classes abstract.

## Muscle memory

Type these until they require no thought:

```cpp
struct Camera : Sensor { ... };                    // ": Sensor" = derives from Sensor
virtual std::string name() const = 0;              // pure virtual: an interface method
virtual ~Sensor() = default;                       // EVERY polymorphic base, on reflex
std::string name() const override { ... }          // every override says override
std::unique_ptr<Sensor> s = std::make_unique<Camera>();  // interface + ownership
void use(const Sensor& s);                         // polymorphic param: by reference
std::vector<std::unique_ptr<Sensor>> all;          // heterogeneous collection: pointers,
                                                   // never vector<Sensor> (slices)
```

## The drills

Open `starter.cpp`; each stub restates its own hints in comments. `destructionLog()`
— a shared `vector<std::string>` that destructors append their class name to — is
provided plumbing so `main()` can assert *which* destructors ran and in what order.

### Drill 1 — `Sensor`: the interface

Task: complete the abstract base class — the two pure virtual declarations are
given; make the virtual destructor append `"Sensor"` to `destructionLog()`.

```cpp
class Sensor {
public:
    virtual ~Sensor() { destructionLog().push_back("Sensor"); }
    virtual std::string name() const = 0;
    virtual double read() = 0;
};
// Sensor s;                          -> compile error: abstract class
// std::unique_ptr<Sensor> ok = std::make_unique<Camera>();   // the intended use
```

Gotcha: the destructor is the only member here with a body — an interface with pure
virtual methods still needs a *virtual, implemented* destructor, because destructors
are the one thing a derived class can never fully take over.

**Where you'll see it:** "design a base class for X" is the C++ OOP interview
archetype, and the hidden scoring criterion is whether the virtual destructor
appears *unprompted* — many interviewers admit it is the first thing they look for.
In real inference code this class is the `Detector` / `InferenceBackend` interface
with TensorRT, ONNX Runtime, and CPU implementations behind it, and in robotics
middleware it is the hardware-abstraction seam (ROS 2 hardware interfaces, camera
driver plugins) that lets the same stack run in sim and on the robot.

### Drill 2 — `Camera` and `Lidar`: two concrete backends

Task: implement both classes — `name()` returns `"camera"` / `"lidar"`, `read()`
returns `30.0` / `10.0` (pretend rates in Hz), and each destructor logs its own
class name before `~Sensor` logs `"Sensor"`.

```cpp
{
    std::unique_ptr<Sensor> s = std::make_unique<Camera>();
}   // destructionLog() -> {"Camera", "Sensor"}: derived ran FIRST, base second.
    // With a non-virtual ~Sensor it would log just {"Sensor"} — the leak bug,
    // caught by an assert instead of a GPU out-of-memory at 2 a.m.
```

Gotcha: write `override` on all six members, including the destructors — and note
that the destructors *log in the order they run*, which is what turns "trust me,
derived runs first" into an assertable fact.

**Where you'll see it:** the follow-up to every interface question is "implement
one, and tell me what happens when it's destroyed through the base pointer" — the
non-virtual-destructor question is a genuine top-5 C++ interview classic, asked at
every level. The camera + lidar pair is the canonical heterogeneous-sensor setup in
AV/robotics stacks, where each driver holds real resources (USB handles, DMA
buffers) that must be released by *its own* destructor.

### Drill 3 — `pollAll(sensors)`: one loop, zero type checks

Task: `pollAll(const std::vector<std::unique_ptr<Sensor>>&)` returns a
`std::vector<std::string>`, one `"name=value"` line per sensor, via
`std::ostringstream`.

```cpp
// sensors = {Camera, Lidar, Camera}
pollAll(sensors)   // -> {"camera=30", "lidar=10", "camera=30"}
// ostringstream prints 30.0 as "30" — default formatting drops the ".0".
// The loop body mentions ONLY Sensor. No casts, no type switches:
// each s->name() / s->read() lands in the right class via the vtable.
```

Gotcha: loop with `const auto& s` — `unique_ptr` cannot be copied (lesson 02), so
`for (auto s : sensors)` does not compile.

**Where you'll see it:** "process a heterogeneous collection without checking
types" is the standard polymorphism exercise, and interviewers read a
`dynamic_cast`-free loop as the pass signal. It is also the actual main loop of a
robot: iterate registered sensors, poll each through the interface — and inside an
inference engine, the same shape runs a network as `for (op : graph) op->execute()`
at operator granularity (where "The road ahead" below says virtual is the right
tool).

### Drill 4 — `describe(const Sensor&)`: polymorphism by reference

Task: return `"Sensor[" + name + "]"` for any sensor, taking the parameter by
`const Sensor&`.

```cpp
Camera cam;
describe(cam)              // -> "Sensor[camera]"
const Sensor& asBase = cam;
describe(asBase)           // -> "Sensor[camera]"  still the Camera underneath
// By VALUE — std::string describe(Sensor s) — this exact code would not even
// compile: Sensor is abstract, and a by-value parameter IS an instantiation.
// With a concrete base it would compile and slice (§7). Reference = correct always.
```

**Where you'll see it:** interviewers hand you `void print(Shape s)` plus a
surprising output and ask what went wrong — slicing is *the* "spot the bug" staple
for C++ OOP rounds, and by-value parameters are its favorite hiding place. In real
code this function is every logger, telemetry formatter, and debug-dump helper that
accepts "any sensor / any model" — all written against `const Base&`.

### Drill 5 — `brokenDispatchDemo()`: the forgotten `virtual`

Task: `BrokenBase` / `BrokenDerived` are given, with a deliberately non-virtual
`id()`. Construct a `BrokenDerived`, view it through a `const BrokenBase*`, return
`p->id()` — and *predict the result before you run it*.

```cpp
BrokenDerived d;
const BrokenBase* p = &d;
p->id()   // -> "BrokenBase". The object is a BrokenDerived; the POINTER type
          // decided at compile time. No virtual, no vtable, no runtime look-up.
```

Gotcha: nothing here is wrong to the compiler — the assert enshrines behavior that
builds cleanly everywhere and surprises almost everyone the first time they meet it.

**Where you'll see it:** "what does this program print?" with a non-virtual method
called through a base pointer is, alongside the destructor question, the most
recycled C++ screener in existence — often both in one snippet. In practice it is a
classic first-week bug in driver-interface code: the program runs, nothing crashes,
and every backend silently executes the base-class stub.

## How to practice

```sh
# Against the reference solution (should pass out of the box):
uv run pytest ramp_up/cpp/04_virtual_functions_and_vtables -v

# Against YOUR implementation in starter.cpp:
PRACTICE=1 uv run pytest ramp_up/cpp/04_virtual_functions_and_vtables -v
```

Or compile and run directly — `main()` asserts every drill and prints
`ALL TESTS PASSED`:

```sh
clang++ -std=c++17 -Wall -o /tmp/vtables starter.cpp && /tmp/vtables
```

## The road ahead

Carry one cost model forward: a virtual call is two loads, an indirect jump, and —
the part that matters — a blocked inliner. That price is invisible when you pay it
rarely and ruinous when you pay it per element. Real inference engines draw the line
exactly where step 3 predicts: the graph executor calls `op->execute()` through an
interface — one virtual call per layer per frame, nothing — while *inside* each
operator, the million-iteration loops over pixels and activations are plain
functions and templates the compiler can inline and vectorize. When we reach CUDA,
the same shape reappears even more starkly: the host picks which kernel to launch
(the flexible, dispatch-y part), and inside the kernel there is no dispatch at all —
just straight-line arithmetic over thousands of threads. Design rule, and interview
sound bite, in four words: **virtual per-layer, never per-pixel.**
