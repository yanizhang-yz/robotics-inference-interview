# 06 — Classes and Dataclasses

After this lesson you can write a Python class that does everything a well-crafted
Java class does — value equality, use as a map key, sorting, iteration, a readable
string form, controlled attribute access — in roughly a tenth of the code. You will
know exactly which one-line Python feature replaces each block of Java ceremony:
`@dataclass` replaces the POJO boilerplate, *dunder methods* replace `implements
Comparable`/`Iterable`/`toString`, `@property` replaces getter/setter pairs,
`@classmethod` replaces static factories, and *duck typing* replaces the interface
itself. Every one of those terms is defined below before you need it.

## The Java you know

This is the class you have written a hundred times — an immutable 2D vector,
done properly:

```java
public final class Vector2D {
    private final double x;
    private final double y;

    public Vector2D(double x, double y) { this.x = x; this.y = y; }

    public double getX() { return x; }
    public double getY() { return y; }

    public Vector2D add(Vector2D o) { return new Vector2D(x + o.x, y + o.y); }

    @Override public boolean equals(Object o) {
        if (this == o) return true;
        if (!(o instanceof Vector2D)) return false;
        Vector2D v = (Vector2D) o;
        return Double.compare(x, v.x) == 0 && Double.compare(y, v.y) == 0;
    }
    @Override public int hashCode() { return Objects.hash(x, y); }
    @Override public String toString() { return "Vector2D(" + x + ", " + y + ")"; }
}
```

Add `implements Comparable<Vector2D>` and a `compareTo` if you want sorting.
Twenty-five lines before any actual logic, and `a.add(b).add(c)` instead of `a + b + c`.
Lombok's `@Value` exists precisely because this hurts. Python builds the equivalent
of Lombok into the standard library, and this lesson is about using it.

## The lesson

### 1. A class with no ceremony

A minimal Python class, next to its Java twin:

```java
// Java
public class Motor {
    private int rpm;
    public Motor(int rpm) { this.rpm = rpm; }
}
Motor m = new Motor(50);
```

```python
# Python
class Motor:
    def __init__(self, rpm):
        self.rpm = rpm      # this line CREATES the field — no declaration above

m = Motor(50)               # no `new` keyword; calling the class constructs it
```

What changed, piece by piece:

- **`__init__` is the constructor** (strictly: the *initializer* — the object already
  exists when it runs, but "constructor" is the right mental model). You never call it
  directly; `Motor(50)` does.
- **`self` is Java's `this`, but explicit.** Every instance method takes it as the
  first parameter, and Python fills it in when you call `m.method()`. Forget it and
  the error is confusing until you've seen it once: defining `def speak():` (no
  `self`) on `Motor` and calling `m.speak()` gives
  `TypeError: Motor.speak() takes 0 positional arguments but 1 was given` —
  the "1" is `self` being passed to a method that didn't declare it (Python prefixes
  the class name, so you'll see `Motor.speak()`, not a bare `speak()`).
- **There are no field declarations.** Assigning `self.rpm = 50` inside `__init__`
  creates the attribute. Corollary gotcha: assignment to a *misspelled* attribute
  (`m.rmp = 90`) does not error — it silently creates a new attribute named `rmp`.
  There is no compiler to catch it; tests catch it.
- **There is no `private` keyword.** The convention is a leading underscore:
  `self._items` means "internal, don't touch" — but nothing enforces it;
  `m._items` works fine. It's a signal to readers, not a lock.

### 2. `@dataclass` — Lombok built into the language

A **decorator** is a function applied to a class or function at definition time,
written `@name` on the line above — Java annotations look the same but are passive
metadata; a Python decorator actively *transforms* what it decorates. `@dataclass`
(from the standard library's `dataclasses` module) reads your field declarations and
generates `__init__`, `__eq__`, and `__repr__` for you — the same job as Lombok's
`@Data`, with no build plugin:

```python
from dataclasses import dataclass

@dataclass
class Task:
    priority: int      # these are type ANNOTATIONS — hints for tools and readers.
    name: str          # Python does NOT enforce them at runtime: Task("a", 3) runs.

t = Task(1, "ship")
print(t)                            # -> Task(priority=1, name='ship')
print(Task(1, "ship") == Task(1, "ship"))   # -> True  (field-by-field equality)
```

Three things Java makes you earn, shown in those four lines:

- **Value semantics for free.** *Value semantics* means two objects are equal when
  their contents are equal, regardless of being distinct objects in memory — what you
  get in Java only after hand-writing `equals()`. A plain (non-dataclass) Python class
  behaves like Java's default: `==` falls back to identity, so
  `Motor(50) == Motor(50)` is `False` (two distinct objects with equal contents).
  `@dataclass` writes the field-comparing `__eq__` for you.
- **`==` vs `is`.** In Python, `==` always calls the `__eq__` method (Java's
  `.equals()`), and `is` compares object identity (Java's `==` on references). This
  is inverted from Java muscle memory — see Gotcha 2 in
  [`../LEARNING_POINTS.md`](../LEARNING_POINTS.md).
- **A debugger-ready string form.** The generated `__repr__` prints as the
  constructor call: `Task(priority=1, name='ship')`. More on `__repr__` below.

**`frozen=True` — the immutable value type.** *Immutable* means no field can be
reassigned after construction (Java: all fields `final`). It also solves hashing:

```python
@dataclass(frozen=True)
class Vector2D:
    x: float
    y: float

v = Vector2D(1.0, 2.0)
v.x = 9.0                                     # -> FrozenInstanceError: cannot assign to field 'x'
print(len({Vector2D(1.0, 2.0), Vector2D(1.0, 2.0)}))   # -> 1 (duplicates collapse)
```

*Hashable* means an object can be reduced to a stable integer (its *hash*) that
dicts and sets use to pick a storage bucket — exactly what `hashCode()` is for.
The Java contract "if you override `equals`, override `hashCode` consistently"
exists in Python too, and `frozen=True` generates both correctly, together.

**The gotcha:** a *non*-frozen dataclass is deliberately **unhashable** — its fields
can change, which would corrupt any set or dict holding it (the same reason mutating
a Java `HashMap` key loses the entry). Try `{Task(1, "ship")}` and you get
`TypeError: unhashable type: 'Task'`. If you want dict-key/set behavior, reach for
`frozen=True`.

### 3. Dunder methods — how Python does "interfaces"

A **dunder method** ("double underscore", e.g. `__add__`, `__len__`) is a method
with a reserved name that the *language itself* calls when you use an operator or
builtin. You define `__add__`; users write `+`. This is **operator overloading** —
defining what an operator means for your own type — which Java forbids (only `+` on
`String` is special-cased, which is why `BigDecimal` arithmetic reads as
`a.add(b).multiply(c)`).

```python
@dataclass(frozen=True)
class Vector2D:
    x: float
    y: float

    def __add__(self, other):                     # this is what `+` calls
        return Vector2D(self.x + other.x, self.y + other.y)

print(Vector2D(1.0, 2.0) + Vector2D(3.0, 4.0))    # -> Vector2D(x=4.0, y=6.0)
```

Use an operator on a type that doesn't define its dunder and you get
`TypeError: unsupported operand type(s) for +: 'NoAdd' and 'NoAdd'`.

**`__repr__` is `toString()` with a convention.** `repr(obj)` is the developer-facing
string (logs, debugger, test failures); the convention is output that looks like the
constructor call, so you can copy-paste it back into code: `Vector2D(x=1.0, y=2.0)`.
Dataclasses generate exactly that. There is also `__str__` for user-facing display;
if you don't define it, `str(obj)` and `print(obj)` fall back to `__repr__`, so
`__repr__` alone is enough (define it on every class — an unhelpful default like
`<__main__.Motor object at 0x104f3ce90>` is what you get otherwise).
Inside an **f-string** (Python's interpolated string literal — `f"rpm={rpm}"`
embeds the value in place, like `String.format` without the ceremony), the `!r`
conversion means "use the repr":
`f"Dog(name={name!r})"` -> `Dog(name='Rex')` — note the quotes around Rex, which
is why the tests can distinguish a string field from its bare value.

**The gotcha:** dunders like `__add__` on an immutable type must **return a new
object**, never mutate. If your `__add__` tries `self.x += other.x` on a frozen
dataclass, you get `FrozenInstanceError`; on a mutable class you'd silently corrupt
the left operand of every `+`.

### 4. `@property` — getter/setter pairs, retrofitted only when needed

Java drills "always write getters, even trivial ones" because a public field can
never later gain logic without breaking every caller (`obj.field` cannot become
`obj.getField()` at call sites). Python removes the reason for that rule: a plain
public attribute can be *upgraded in place* to a computed one, and callers never
notice, because both use the same syntax.

```java
// Java: the ceremony is mandatory up front
private double celsius;
public double getFahrenheit()        { return celsius * 9 / 5 + 32; }
public void   setFahrenheit(double f) { this.celsius = (f - 32) * 5 / 9; }
```

```python
class Temperature:
    def __init__(self, celsius):
        self.celsius = celsius            # plain public attribute — idiomatic

    @property                             # turns the method into a READ of `t.fahrenheit`
    def fahrenheit(self):
        return self.celsius * 9 / 5 + 32

    @fahrenheit.setter                    # turns this one into a WRITE of `t.fahrenheit = ...`
    def fahrenheit(self, value):
        self.celsius = (value - 32) * 5 / 9

t = Temperature(100.0)
print(t.fahrenheit)     # -> 212.0    (attribute syntax, method runs)
t.fahrenheit = 32.0     # setter runs, converts, stores
print(t.celsius)        # -> 0.0
```

Notes that save you a debugging session:

- The `@property`-decorated method comes first; `@fahrenheit.setter` decorates a
  *second* method **with the same name**. Yes, the same name twice — that's the syntax.
- Skip the setter and the property is read-only: assigning raises
  `AttributeError: property 'fahrenheit' of 'Temperature' object has no setter`.
- **The gotcha:** don't write `getCelsius`-style methods in Python. Reviewers (and
  interviewers) read a `get_x()`/`set_x()` pair as "Java accent"; use a plain
  attribute until you actually need logic, then `@property`.

### 5. The container protocol — `__len__`, `__bool__`, `__iter__`

In Java, being a container means declaring it: `implements Iterable<T>`, write an
`iterator()` returning a hand-rolled `Iterator<T>`, add `size()` and `isEmpty()`.
In Python, a **protocol** is an informal contract: define the right dunders and
every language feature that consumes that contract works with your type — no
declaration anywhere.

- **`__len__`** makes `len(stack)` work (Java: `size()`).
- **`__bool__`** defines *truthiness* — what your object means in `if stack:`.
  Empty containers being "false" is idiomatic Python (Gotcha 3 in
  [`../LEARNING_POINTS.md`](../LEARNING_POINTS.md)). Defaults matter here: an object
  with neither dunder is **always truthy**; if only `__len__` is defined, Python
  falls back to `len(obj) != 0`. Defining `__bool__` just makes the contract explicit.
- **`__iter__`** must return an **iterator** — an object that hands out one element
  per step until exhausted (Java's `Iterator<T>`, `hasNext`/`next` fused into one
  concept). An **iterable** is anything that can produce an iterator; `for`,
  `list()`, `any()` all accept iterables. You almost never write an iterator class:
  return one you already have. `reversed(self._items)` *is* an iterator
  (`list_reverseiterator`), so a stack that yields top-first is one line.

```python
class Stack:
    def __init__(self):
        self._items = []                      # _underscore: internal by convention

    def push(self, item): self._items.append(item)
    def pop(self):        return self._items.pop()     # IndexError if empty — see below
    def __len__(self):    return len(self._items)
    def __bool__(self):   return bool(self._items)
    def __iter__(self):   return reversed(self._items) # fresh iterator per call

s = Stack(); s.push("a"); s.push("b"); s.push("c")
print(len(s), list(s))    # -> 3 ['c', 'b', 'a']   (top-first, and NOT consumed)
while s:                  # truthiness in action: loop until empty, no isEmpty()
    s.pop()
```

Because `__iter__` returns a *fresh* iterator each call, iterating twice gives the
same elements twice — the stack is not consumed.

**Errors: EAFP over LBYL.** Java pre-checks (`if (stack.isEmpty()) throw new
EmptyStackException()`) — *Look Before You Leap* (LBYL). Python's idiom is *Easier
to Ask Forgiveness than Permission* (EAFP): just do the operation and let the
natural exception fly. `[].pop()` already raises
`IndexError: pop from empty list`, and `[][-1]` raises
`IndexError: list index out of range` — so `pop()`/`peek()` need **no** empty check
at all. (Gotcha 11 in [`../LEARNING_POINTS.md`](../LEARNING_POINTS.md) covers why
exceptions-as-flow is cheap and idiomatic in Python.)

### 6. Ordering — `__lt__` makes `sorted()` and `heapq` accept your type

Java's `Comparable<T>` demands a three-way `compareTo` returning negative/zero/
positive. Python's sorting machinery only ever asks one question — "is a less than
b?" — so you implement one dunder, `__lt__` ("less than"):

```python
@dataclass
class Task:
    priority: int
    name: str

    def __lt__(self, other):
        return self.priority < other.priority

tasks = [Task(3, "clean"), Task(1, "ship"), Task(2, "test")]
print([t.name for t in sorted(tasks)])   # -> ['ship', 'test', 'clean']
```

- Without `__lt__`, `sorted()` fails with
  `TypeError: '<' not supported between instances of 'Task' and 'Task'`.
- **`sorted()` is stable**: elements that compare equal keep their original relative
  order (like Java's `Collections.sort`), so sorting only by priority never
  scrambles same-priority tasks.
- **`heapq` is `PriorityQueue`.** A *heap* is the array-backed tree behind a
  priority queue: the smallest element is always first out. Python's `heapq` module
  operates on a plain list, and like `sorted()` it only needs `<`:
  `heapq.heappush(heap, Task(1, "ship"))` then `heapq.heappop(heap)` returns the
  lowest-priority-number task.
- Shortcut worth knowing: `@dataclass(order=True)` generates `__lt__` (and friends)
  comparing fields *lexicographically* — meaning field by field in declaration
  order, like a `Comparator.comparing(...).thenComparing(...)` chain, or how words
  order in a dictionary. Here we hand-write `__lt__` because the drill orders by
  priority only, ignoring the name.
- **The gotcha:** the common no-class alternative is pushing tuples,
  `heapq.heappush(heap, (priority, task))` — but on a priority *tie* Python compares
  the second elements, and if those aren't comparable you get a surprise
  `TypeError: '<' not supported between instances of 'Job' and 'Job'`.
  Defining `__lt__` on the object (or padding with a counter:
  `(priority, i, task)`) avoids it.

### 7. `@classmethod` — the static factory that knows its class

Java static factories (`Optional.of`, `Integer.valueOf`, `LocalDate.parse`) are
`static` methods that hard-code the class they construct. A Python **classmethod**
receives the class itself as the first parameter — named `cls` by convention, the
class-level mirror of `self` — so it constructs whatever class it was *called on*,
subclasses included:

```python
@dataclass
class Point:
    x: float
    y: float

    @classmethod
    def from_string(cls, s):
        x_str, y_str = s.split(",")       # "3,4" -> ["3", "4"]
        return cls(float(x_str), float(y_str))   # cls, NOT Point — see below

print(Point.from_string(" 1.5, -2.0 "))   # -> Point(x=1.5, y=-2.0)
```

- No manual `.strip()` needed for this drill: `float(" 4 ")` -> `4.0` — `float()`
  tolerates surrounding whitespace.
- **Why `cls(...)` and not `Point(...)`:** if a subclass calls the factory —
  `LabeledPoint.from_string("0,0")` — `cls` *is* `LabeledPoint`, so the subclass
  gets back an instance of itself. Hard-coding `Point(...)` would silently return
  the wrong type. This is the behavior Java static factories can't have without
  reflection tricks.
- Naming convention: `from_<what>` — `from_string`, `from_dict`, `from_config`.

### 8. Duck typing — the interface you never declare

**Duck typing**: "if it walks like a duck and quacks like a duck, it's a duck" —
a function that calls `obj.speak()` accepts *any* object that has a `speak()`
method. There is no interface to declare, no `implements`, no shared base class,
no cast. The method lookup happens at runtime; the "contract" is just the method
existing.

```java
// Java: the contract must be declared before the call site compiles
interface Speaker { String speak(); }
class Dog implements Speaker { public String speak() { return "woof"; } }
static String describe(Speaker s) { return s.speak(); }
```

```python
# Python: no interface exists anywhere in this program
class Dog:
    def speak(self): return "woof"

class Robot:                       # shares NO base class with Dog
    def speak(self): return "beep boop"

def describe(obj):
    return f"{type(obj).__name__}: {obj.speak()}"   # works on ANYTHING with .speak()

print(describe(Dog()))     # -> Dog: woof
print(describe(Robot()))   # -> Robot: beep boop
```

- `type(obj).__name__` is `obj.getClass().getSimpleName()` — the class name as a
  string, handy for reprs and messages.
- **The trade you're making:** Java rejects a non-`Speaker` at compile time; Python
  finds out at the call — `describe(Cat())` on a speechless `Cat` raises
  `AttributeError: 'Cat' object has no attribute 'speak'` *at runtime*. That's the
  deal across all of Python: flexibility now, errors at execution. (Static type
  checkers can restore compile-style checking with `typing.Protocol` — an interface
  matched by shape rather than by declaration — but nothing in this drill needs it.)

## Muscle memory

Type these without thinking:

```python
@dataclass(frozen=True)                    # immutable value type: init/eq/repr/hash free
class Pose:
    x: float
    y: float

def __repr__(self):                        # constructor-call convention, !r for quotes
    return f"Dog(name={self.name!r})"

def __add__(self, other):                  # a + b on your type; return a NEW object
    return Vector2D(self.x + other.x, self.y + other.y)

@property                                  # read...
def fahrenheit(self): ...
@fahrenheit.setter                         # ...write, SAME method name
def fahrenheit(self, value): ...

def __lt__(self, other):                   # unlocks sorted() and heapq
    return self.priority < other.priority

def __iter__(self):                        # reuse an existing iterator
    return iter(self._items)

@classmethod
def from_string(cls, s):                   # factory; cls(...) not ClassName(...)
    return cls(...)

type(obj).__name__                         # getClass().getSimpleName()
```

## The drills

### `Vector2D`

Build an immutable 2D vector where `v1 + v2`, `v1 - v2`, `.scaled(k)`, and
`.magnitude()` work, and instances behave in sets/dicts.

```python
@dataclass(frozen=True)
class Vector2D:
    x: float
    y: float
    def __add__(self, other):
        return Vector2D(self.x + other.x, self.y + other.y)

Vector2D(1.0, 2.0) + Vector2D(3.0, 4.0)   # -> Vector2D(x=4.0, y=6.0)
math.hypot(3.0, 4.0)                       # -> 5.0  (magnitude, no manual sqrt)
```

**Where you'll see it:** the frozen-value-type pattern is the Python answer to the
classic Java interview question "make this class safe as a `HashMap` key" — the
`equals`/`hashCode` contract, generated. Grid-search problems ("Number of Islands",
"Rotting Oranges", any BFS/DFS over coordinates) need positions in a `visited` set;
most solutions use bare tuples, but a frozen `Pose`/`Point` dataclass is the
readable production version — robotics code is full of exactly this class
(waypoints, joint states, gripper poses). Operator dunders are also *why*
`a + b` works on numpy arrays and torch tensors: those libraries define `__add__`.

### `Temperature`

Store `celsius` as a plain attribute; expose `fahrenheit` as a computed, settable
attribute via `@property` + setter.

```python
t = Temperature(0.0)
t.fahrenheit = 212.0      # setter converts and stores
t.celsius                 # -> 100.0
```

**Where you'll see it:** rarely a whiteboard algorithm — this is the idiom that
code-review and "design a class" rounds check, and Java-refugee screens
specifically look for the anti-pattern (`get_x()`/`set_x()` pairs). In real
ML/robotics code, `@property` is everywhere: computed fields on configs, derived
quantities on state objects (`robot.gripper_open`, `dataset.num_frames`), setters
that validate or clamp (a servo-angle setter refusing values outside its range).

### `Stack`

A LIFO (last-in-first-out) stack over a private list where `len(s)`, `if s:`, and `for item in s:`
(top-first) all work, and empty `pop()`/`peek()` raise `IndexError` with no
pre-check (EAFP).

```python
s = Stack(); s.push(1); s.push(2)
len(s), bool(s), list(s)   # -> (2, True, [2, 1])
Stack().pop()              # -> IndexError: pop from empty list
```

**Where you'll see it:** the stack is the most-tested data structure in screens —
"Valid Parentheses", "Min Stack", "Evaluate Reverse Polish Notation", the
"monotonic stack" family — a stack you keep sorted by popping smaller/larger
elements before each push ("Daily Temperatures", "Largest Rectangle in
Histogram") — and iterative DFS. Interviewers read `if stack:` vs `if len(stack) > 0` as a
direct fluency signal. The dunder-protocol part is what makes your own robotics
types (a trajectory, a joint chain) work in `for` loops and `len()` like builtins.

### `Task`

Give a dataclass `__lt__` ordering by `priority` so `sorted()` and `heapq` accept
it directly.

```python
sorted([Task(3, "clean"), Task(1, "ship")])[0].name   # -> 'ship'
heapq.heappush(heap, Task(1, "ship"))                 # no wrapper tuple needed
```

**Where you'll see it:** every heap problem — "Merge k Sorted Lists", "Top K
Frequent Elements", "K Closest Points to Origin", "Kth Largest Element in an
Array", "Task Scheduler", and Dijkstra-style shortest paths ("Network Delay
Time"). Knowing that `heapq` needs only `<` — and that `(priority, obj)` tuples
blow up on ties when `obj` isn't comparable — is exactly the kind of detail that
separates "learned Python last week" from fluent. In robotics: A* open sets in
planners, event/timer queues in simulators and schedulers.

### `Point.from_string`

Parse `"3,4"` (whitespace tolerated) into a `Point` via a `@classmethod` factory
that constructs with `cls(...)`.

```python
Point.from_string(" 1.5, -2.0 ")   # -> Point(x=1.5, y=-2.0)
class LabeledPoint(Point): pass
type(LabeledPoint.from_string("0,0")).__name__   # -> 'LabeledPoint'
```

**Where you'll see it:** interview inputs frequently arrive as strings to parse
("moves given as `'U,D,L,R'`", coordinate lists, log lines) — the
split-and-convert reflex should be instant. As a design pattern, alternate
constructors are the standard ML-library idiom: Hugging Face's
`Model.from_pretrained(...)`, `datetime.fromtimestamp(...)`, `dict.fromkeys(...)`,
lerobot-style `Config.from_dict(...)`. When an interviewer asks "how would you add
a second constructor in Python?" (there's no overloading), `@classmethod` factory
is the expected answer.

### `describe()` + `Dog` and `Robot`

Write two unrelated classes with `speak()` and one function that formats
`"<ClassName>: <speech>"` for *any* object that speaks — no shared base, no
interface.

```python
describe(Dog("Rex"))     # -> 'Dog: Rex says woof'
describe(Robot("R2"))    # -> 'Robot: R2 goes beep boop'
```

**Where you'll see it:** this is the conceptual question Java-background
candidates get asked directly — "Python has no interfaces; how do you program to a
contract?" Duck typing is the answer, and being able to articulate the trade
(flexibility vs. runtime `AttributeError`) matters as much as the code. In the
wild: file-like objects (anything with `.read()` passes as a file), test mocks
that stand in for real hardware drivers, and PyTorch's convention that any object
with the right methods slots into a training loop.

## How to practice

Write your attempts in `starter.py`, then run the tests against them:

```bash
# Against the reference solutions (should pass out of the box):
uv run pytest ramp_up/python/06_classes_and_dataclasses -v

# Against YOUR implementation in starter.py:
PRACTICE=1 uv run pytest ramp_up/python/06_classes_and_dataclasses -v
```

The tests check the exact `__repr__` strings listed at the top of `starter.py` —
if a repr test fails, compare quotes and spacing character by character (`!r`
is usually the missing piece). For the language-wide background (gotchas, the
Java-to-Python rosetta table, interview idioms), see
[`../LEARNING_POINTS.md`](../LEARNING_POINTS.md).
