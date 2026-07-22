# 04 — Virtual Functions and Vtables: inheritance, `override`, and the virtual destructor

In Java, polymorphism is the air you breathe: every method call runs the object's
real version, always, and you have never once thought about it. In C++ that behavior
is **opt-in** — and forgetting to opt in doesn't produce an error, it silently runs
the *wrong function*. After this lesson you will be able to: predict exactly which
function body runs for any call through a pointer, a reference, or a plain value;
write a C++ interface (abstract base class) the way production code does it; explain
what a vtable is in one breath; and nail the two classic interview traps — the
non-virtual destructor and object slicing — that Java made impossible to even write.
Every term is defined the first time it appears; every snippet's output was verified
with `clang++ -std=c++17 -Wall`.

## The Java you know

```java
class Animal {
    String speak() { return "..."; }
}
class Dog extends Animal {
    @Override String speak() { return "woof"; }  // @Override = typo insurance
}

Animal a = new Dog();
a.speak();                    // -> "woof". ALWAYS the object's real class.
// You cannot turn this off. No Java program exists in which a.speak()
// runs Animal's version while a points at a Dog.

interface Sensor {            // a pure contract: no fields, no bodies
    String name();
    double read();
}

// Cleanup: the GC eventually frees the Dog. "Which destructor runs?" is not
// a question Java lets you ask — there are no destructors.
```

Three certainties are hiding in that snippet: dynamic dispatch is automatic,
`@Override` is optional insurance, and teardown is Somebody Else's Problem. In C++
all three are decisions you make — and each wrong decision compiles cleanly and
misbehaves at runtime. That is this lesson.

## The lesson

### 1. Without `virtual`, the pointer's type picks the function — at compile time

First, the inheritance syntax itself: `struct Robot : Machine` is Java's
`class Robot extends Machine`. (With the `class` keyword you write
`class Robot : public Machine` — members and base classes default to `private` in a
`class` and `public` in a `struct`; that is the only difference between the two
keywords.) Upcasting works like Java: a `Robot` **is a** `Machine`, so a `Machine*`
(pointer) or `Machine&` (reference) may point at one.

Now the shock. Here is the most Java-looking C++ possible — and it does the wrong
thing:

```cpp
#include <iostream>
#include <string>

struct Machine {
    std::string name() const { return "Machine"; }   // note: no virtual keyword
};
struct Robot : Machine {
    std::string name() const { return "Robot"; }
};

int main() {
    Robot r;
    std::cout << r.name() << "\n";     // -> Robot     fine so far

    Machine* p = &r;                   // legal upcast, exactly like Java
    std::cout << p->name() << "\n";    // -> Machine   (!!) the object IS a Robot

    Machine& ref = r;
    std::cout << ref.name() << "\n";   // -> Machine   references: same story
}
```

What happened: C++ has two ways to decide which function body a call runs.
**Static dispatch** means the *compiler* picks the function at compile time, looking
only at the *declared type* of the expression — `p` is declared `Machine*`, so
`p->name()` is hard-wired to `Machine::name` before the program ever runs. The
object's real class is never consulted. **Dynamic dispatch** means the decision is
made at *runtime* from the object's actual class — that is the only behavior Java's
instance methods have. C++'s default is static dispatch.

And `Robot::name` above does not override anything. It **hides** `Machine::name`:
the two are unrelated functions that happen to share a name, and which one you get
depends entirely on the type of the expression you call through. No error, no
warning in default builds, no exception — just the wrong answer.

The nearest thing Java has to this behavior is `static` methods, which also
dispatch on the declared type — and Java teachers spend a whole lecture warning you
about exactly that. In C++, *everything* works that way until you say otherwise.

### 2. `virtual` opts in to Java's behavior

One keyword on the base-class function restores everything you expect:

```cpp
struct Machine {
    virtual std::string name() const { return "Machine"; }   // opt in HERE
};
struct Robot : Machine {
    std::string name() const override { return "Robot"; }    // override: next section
};

Robot r;
Machine* p = &r;
std::cout << p->name() << "\n";    // -> Robot    Java behavior restored
Machine& ref = r;
std::cout << ref.name() << "\n";   // -> Robot
```

`virtual` on `Machine::name` says: calls to `name` through a `Machine*` or
`Machine&` are resolved at runtime from the object's real class. Virtual-ness is
inherited — once a function is virtual in the base, every derived function with the
same signature is automatically virtual too, all the way down, whether or not the
derived class repeats the keyword.

#### How it works: the vtable

The mechanism is worth knowing because interviewers ask for it by name. A
**vtable** (virtual table) is a hidden per-*class* table of function pointers — one
slot per virtual function, filled with the most-derived version for that class.
`Machine`'s table has `Machine::name` in the slot; `Robot`'s table has
`Robot::name`. Every *object* of a class with at least one virtual function carries
one hidden pointer — the **vptr** — to its class's table, planted by the
constructor. A virtual call compiles to: follow the object's vptr, index the slot,
call whatever is there. The pointer you called through never matters again; the
object brought its own dispatch table.

The vptr is real and you can see it — objects get bigger:

```cpp
struct Plain    { int x; std::string tag() const { return "plain"; } };
struct WithVptr { int x; virtual std::string tag() const { return "virt"; } };

sizeof(Plain)      // -> 4   just the int
sizeof(WithVptr)   // -> 16  int + hidden 8-byte vptr + alignment padding
                   //        (values from clang on a 64-bit machine)
```

Java anchor: the JVM maintains exactly this machinery for every non-static method
of every class — you have been paying for vtables your whole career without seeing
them. C++ just makes the cost visible and optional.

One rule to carry out of this section: dynamic dispatch needs **both** halves —
`virtual` on the function, *and* a pointer or reference at the call site. Lose
either one and you are back to static dispatch. (What happens when you lose the
pointer/reference half is section 6, and it is nastier than it sounds.)

### 3. `override` — `@Override`, with teeth

`override`, written after the parameter list, tells the compiler: "I intend this to
override a base-class virtual — fail the build if it doesn't." It exists because
hiding (section 1) makes signature typos silent. Watch a one-character-class bug:

```cpp
struct Machine {
    virtual std::string name() const { return "Machine"; }
};
struct Robot : Machine {
    std::string name() { return "Robot"; }   // forgot const!
};

Robot r;
Machine* p = &r;
std::cout << p->name() << "\n";   // -> Machine
```

`name() const` and `name()` are *different signatures*, so the derived function is
a brand-new function that hides the base one — the virtual slot still holds
`Machine::name`, and every call through a base pointer quietly runs the base
version. (Clang with `-Wall` does flag this one — `warning: 'Robot::name' hides
overloaded virtual function` — but it is a warning, the program still builds and
still answers wrong; gcc says nothing without extra flags.) Same trap for a
misspelled name, a `float` parameter where the base says `double`, and every other
near-miss.

Add `override` and the near-miss becomes a hard error at the exact line:

```cpp
struct Robot : Machine {
    std::string name() override { return "Robot"; }
    // error: non-virtual member function marked 'override' hides virtual
    //        member function
    // note:  different qualifiers ('const' vs unqualified)
};
```

Rule: **every** overriding function gets `override`, no exceptions — it is
`@Override` upgraded from "recommended annotation" to "compile-time proof." Style
note: in the derived class write `override` *instead of* repeating `virtual`
(`override` already implies it); the drills follow that convention.

### 4. The classic: the virtual destructor

You met destructors in lesson 02: a destructor is the function that runs, at a
deterministic line, when an object dies — and in RAII code it is where files close,
locks release, and memory frees. Inheritance adds a question Java never let you
ask: *when an object dies through a base-class pointer, which destructor runs?*

Set the stage with the correct, everyday case — a stack object dies at its brace
and tears down completely:

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

Note the order: derived destructor body first, then the derived class's *members*,
then the base — the exact reverse of construction. All good. Now the same class
used polymorphically, the way every plugin registry and model zoo uses it:

```cpp
Model* p = new TrtModel;   // or handed to you by a factory
delete p;                  // prints: ~Model
                           // ...and NOTHING else. ~TrtModel never ran.
                           // buf's destructor never ran. GPU memory LEAKED.
```

Why: a destructor is a member function like any other, and this one is not
virtual — so `delete p` static-dispatches on the pointer's type and runs *only*
`Model::~Model`. The derived half of the object is never torn down. (Formally the
C++ standard declares this **undefined behavior** — the program is allowed to do
anything at all; see gotcha 11 in [`../LEARNING_POINTS.md`](../LEARNING_POINTS.md).
What clang and gcc actually do is what you see above: base destructor only, leak
included, no diagnostic.)

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
destructor, which then runs the chain in reverse-construction order.

Memorize the rule as an interview sound bite: **any class with virtual functions
gets a virtual destructor** — if code deletes derived objects through base
pointers, it is mandatory. When the base has no cleanup of its own, write
`virtual ~Model() = default;` (that is `= default` from lesson 02: "generate the
usual body"). Two follow-ups interviewers love:

- `std::unique_ptr<Model>` does **not** save you — at scope end it performs
  `delete` on a `Model*`, exactly the broken call above. Smart pointer, same rule.
- Java has no equivalent bug: the GC frees the *whole object* regardless of the
  variable's declared type. "Partial destruction" is not a thing you could ever
  have written.

### 5. Pure virtual functions and abstract classes — Java interfaces

Sometimes the base version of a function has no sensible body — what would a
generic `Sensor::read()` even return? C++ lets you declare the slot and refuse to
fill it. A **pure virtual function** is a virtual function marked `= 0`, meaning "no
body here; deriving classes must provide one." A class with at least one pure
virtual function is an **abstract class**, and the compiler refuses to instantiate
it:

```cpp
struct Sensor {
    virtual ~Sensor() = default;             // section 4's rule, applied on reflex
    virtual std::string name() const = 0;    // = 0 makes it pure virtual
    virtual double read() = 0;
};

Sensor s;   // error: variable type 'Sensor' is an abstract class
            // note:  unimplemented pure virtual method 'name' in 'Sensor'
```

The Java mapping is direct. All methods pure virtual, no fields → that is an
`interface`. A mix of implemented and pure virtual methods → that is an
`abstract class`. C++ has no separate `interface` keyword and no `implements` —
plain inheritance is the single mechanism for both, and a class may inherit from
several bases at once (that is how "implements two interfaces" is spelled; the
sharp edges of multiple inheritance are a story for another lesson).

A derived class must override *every* pure virtual function or it remains abstract
itself. Once `Camera` overrides both `name()` and `read()`, it is **concrete** —
instantiable — and the idiomatic way to use the family is the one you already know
from lesson 02:

```cpp
std::unique_ptr<Sensor> s = std::make_unique<Camera>();   // interface + ownership
s->read();                                                // dynamic dispatch
// scope end: ~Camera runs, then ~Sensor — because ~Sensor is virtual
```

### 6. Object slicing — the trap Java cannot even express

Recall the #1 mindset shift from [`../LEARNING_POINTS.md`](../LEARNING_POINTS.md):
in Java, a variable of class type is always a *reference* — `Animal a = dog;`
copies a small handle, and the Dog object stays whole on the heap. In C++, a
variable of class type **is** the object, and assignment copies the object itself.
Now combine that with inheritance and ask: copy *into what*? A `Machine` variable
has room for exactly a `Machine`. So:

```cpp
Robot r;                           // the virtual Machine/Robot from section 2
Machine m = r;                     // compiles fine! copies ONLY the Machine part
std::cout << m.name() << "\n";     // -> Machine ... even though name() is virtual
```

This is **object slicing**: initializing or assigning a base-class *value* from a
derived object copies just the base sub-object and throws the derived part away.
The `Robot`-ness didn't come along and get suppressed — it was never copied. `m` is
a genuine, complete `Machine`, its vptr points at `Machine`'s vtable, and dynamic
dispatch is working perfectly on the wrong object. `virtual` cannot help here;
there is nothing left to dispatch to.

The version that gets people in interviews (and code review) is the by-value
parameter, because it *looks* polymorphic:

```cpp
std::string describe(Machine m)        { return m.name(); }   // by VALUE: slices
std::string describe2(const Machine& m) { return m.name(); }  // by REFERENCE: dispatches

Robot r;
describe(r);    // -> "Machine"   r was sliced into the parameter
describe2(r);   // -> "Robot"     a reference to the real object — like a Java parameter
```

Same trap one more way: `std::vector<Machine>` stores `Machine` *values*, so
`v.push_back(r)` slices — `v[0].name()` returns `"Machine"` (verified). A
heterogeneous collection must hold pointers, which is why the drills (and all
production code) use `std::vector<std::unique_ptr<Sensor>>`.

Rule: **polymorphic types travel by pointer or by reference — never by value.**
And a pleasant bonus of section 5: if the base class is abstract, by-value
parameters and `vector<Base>` *don't compile* (you can't instantiate an abstract
class), so a pure-virtual interface turns this whole silent-trap section into loud
compile errors. One more reason real codebases keep their base classes abstract.

### 7. What a virtual call costs — why inference hot loops avoid it

A short aside, because for a robotics-inference role this is the expected closing
sentence of any virtual-functions answer.

A non-virtual call's target is known at compile time, so the compiler can
**inline** it — paste the function's body straight into the call site — and then
keep optimizing across the seam (constant-folding, vectorizing the surrounding
loop). A virtual call is: load vptr → load slot → call *whatever address that is*.
The two loads are nearly free; the real cost is that an unknown target **blocks
inlining**, which blocks every optimization that inlining would have unlocked.
(Java note: the JVM's JIT watches the running program and freely devirtualizes and
inlines call sites that keep hitting one class; a C++ compiler must prove the
target statically and usually can't — so C++ programmers choose at design time.)

The working rule in inference engines: virtual dispatch at *operator* granularity
is fine — one virtual call per layer per frame is nothing. Virtual dispatch
*per element inside the hot loop* — per pixel, per point, per activation — is a
design smell; those loops use plain functions or compile-time polymorphism via
templates (you will hear the acronym **CRTP** for the classic pattern; recognizing
it is plenty for now). Interview sound bite: *"virtual per-layer, never
per-pixel."*

## Muscle memory

Type these until they require no thought:

```cpp
struct Camera : Sensor { ... };                    // ": Sensor" = extends/implements
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
at operator granularity (where section 7 says virtual is the right tool).

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
// With a concrete base it would compile and slice (§6). Reference = correct always.
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
builds cleanly everywhere and is exactly what Java trained you not to expect.

**Where you'll see it:** "what does this program print?" with a non-virtual method
called through a base pointer is, alongside the destructor question, the most
recycled C++ screener in existence — often both in one snippet. In practice it is
the day-one bug of every Java developer writing a C++ driver interface: the code
runs, nothing crashes, and every backend silently executes the base-class stub.

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

Deep dives referenced above — value vs reference semantics (§1) and undefined
behavior (§11) — live in [`../LEARNING_POINTS.md`](../LEARNING_POINTS.md).
