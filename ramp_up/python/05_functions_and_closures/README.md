# 05 — Functions and Closures

After this lesson you will be able to pass functions around like ordinary variables,
build functions that manufacture other functions, replace Java's method overloading
with default and keyword arguments, write a decorator from scratch, handle errors the
Python way (`try` first, check later), and sort anything by any combination of fields
with a one-line key function. These are not exotic tricks — they are the baseline
idioms an interviewer expects to see flow out of your fingers without pauses.

## The Java you know

In Java, "a piece of behavior" is never naked. To pass it anywhere you wrap it in a
**functional interface** — an interface with exactly one abstract method, like
`Function<T,R>` or `Supplier<T>` — and invoke it through that method:

```java
import java.util.function.Function;
import java.util.function.Supplier;
import java.util.concurrent.atomic.AtomicInteger;
import java.util.Comparator;

Function<Integer, Integer> doubler = x -> x * 2;
int y = doubler.apply(5);                        // -> 10; always .apply(), never doubler(5)

// A lambda may only READ locals, and only if they are effectively final
// (never reassigned). To mutate captured state you smuggle it in a box:
AtomicInteger count = new AtomicInteger();
Supplier<Integer> counter = count::incrementAndGet;
counter.get();                                   // -> 1
counter.get();                                   // -> 2

// Optional parameters = overloading (two methods, same name):
double safeDivide(double a, double b) { return safeDivide(a, b, 0.0); }
double safeDivide(double a, double b, double dflt) { return b == 0 ? dflt : a / b; }

// Multi-field ordering = a Comparator chain:
tasks.sort(Comparator.comparing(Task::priority).thenComparing(Task::name));
```

Every one of those five patterns has a shorter Python replacement. That is this lesson.

## The lesson

### 1. Functions are ordinary objects

In Python a function is a **first-class value**: a normal object you can assign to a
variable, put in a list or dict, pass as an argument, and return from another
function — exactly like an `int` or a string. There is no functional-interface
wrapper and no `.apply()`; if `f` holds a function, `f(x)` calls it. A function that
takes or returns another function is called a **higher-order function** (that's the
whole definition — nothing mystical).

```java
// Java: behavior must be boxed before it can travel
Function<String, Integer> f = String::length;
int n = f.apply("hello");                        // -> 5
```

```python
# Python: the function IS the value
f = len                       # no parentheses = the function itself, not a call
f("hello")                    # -> 5

handlers = {"stop": stop, "home": home}   # a dispatch table: dict of functions
handlers["home"]()            # -> calls home(); Java needs a Map<String, Runnable>
```

The gotcha: `f = len()` with parentheses *calls* `len` (and here crashes with a
TypeError); `f = len` without parentheses stores it. If you ever see
`<function foo at 0x...>` printed where you expected a result, you stored the
function and forgot the `()` — or the reverse: you called it too early.

Arity — the number of arguments a function takes — is checked when you *call*, not
when you assign. Passing a 1-argument function where 2 arguments are supplied fails
at the call site with `TypeError: <lambda>() takes 1 positional argument but 2 were
given`, not at "compile time". There is no compiler to catch it earlier.

### 2. `lambda` — the one-expression function literal

`lambda x: x * 2` is an anonymous function, like Java's `x -> x * 2`, with one hard
limit: the body must be a **single expression** (something that produces a value — no
statements, no `if:` blocks, no loops, no assignments). Need more than one line? Use
a named `def`; Python style actually prefers `def` for anything non-trivial.

```python
double = lambda x: x * 2      # works, but for a NAMED function prefer: def double(x): ...
sorted(words, key=lambda w: len(w))   # the natural habitat of lambda: inline key functions
```

### 3. Closures — a function that brings its own variables

You already have the two facts this section is built from. From section 1: **a
function is an object** that you can store in a variable and return from another
function. From section 2: **`()` is what runs it** — until then it's just a value
being passed around. This section adds one new fact — a function can be defined
*inside* another function — and then combines the three. Take it in four small
steps; every snippet is runnable, so type them in as you read.

#### Step 1 — a function inside a function (nothing new yet)

In Java, methods live in classes, never inside other methods. In Python you can
write a `def` inside a `def`, and the inner function may use the outer function's
variables:

```python
def greet_team():
    team = "Weld"
    def greet():                   # a function defined INSIDE another function
        print("Hello, " + team)    # it may use greet_team's variables
    greet()                        # call it while greet_team is still running

greet_team()                       # -> Hello, Weld
```

No surprises: while `greet_team` is running, `team` exists, so the inner function
can read it. So far this is just tidier scoping.

#### Step 2 — return the inner function instead of calling it

Functions are values — so the outer function can hand the inner one back to the
caller instead of running it:

```python
def make_greeter(name):
    def greet():
        return "Hello, " + name
    return greet                   # NO parentheses: return the function itself, un-run

hello_ann = make_greeter("Ann")
hello_bob = make_greeter("Bob")
hello_ann()                        # -> 'Hello, Ann'
hello_bob()                        # -> 'Hello, Bob'
hello_ann()                        # -> 'Hello, Ann'   works every time, not just once
```

Play the first call in slow motion, like stepping through a debugger:

1. `make_greeter("Ann")` starts running. Its parameter `name` holds `"Ann"`.
2. The `def greet():` line runs. Running a `def` does **not** run the body — it
   just *creates a function object* (section 1).
3. `return greet` hands that new object back; it lands in `hello_ann`.
4. `make_greeter` finishes.
5. Later, `hello_ann()` runs the body at last — and the body needs `name`.
   It finds `"Ann"`.

Step 5 should bother you. In Java, a method's local variables stop existing the
moment the method returns — and `make_greeter` returned back in step 4. Where did
`name` survive?

#### Step 3 — the answer: the function packs a backpack

At step 2 — the moment Python creates the inner function object — it looks at the
inner body, notices which of the outer function's variables it uses (here: `name`),
and **packs those variables into the function object itself**. Think of a small
backpack zipped onto `greet`. Wherever the function object travels, the backpack
travels with it; when the body finally runs and needs `name`, it looks in the
backpack.

That's the whole secret, and it has a name: a **closure** is an inner function
carrying a backpack of variables from the function it was born in. Nothing more.

Two consequences, both already visible above:

- **One backpack per factory call.** `make_greeter("Ann")` and `make_greeter("Bob")`
  each created their *own* `greet` with its *own* backpack — which is why the two
  greeters never interfere.
- **The backpack is permanent.** Call `hello_ann()` a thousand times, tomorrow, from
  another file — `"Ann"` rides along.

You have actually built this machine in Java many times — as a tiny class:

```java
class Greeter {
    private final String name;                    // <- the backpack
    Greeter(String name) { this.name = name; }    // <- make_greeter(name)
    String greet() { return "Hello, " + name; }   // <- the inner function
}
```

`make_greeter("Ann")` plays the role of `new Greeter("Ann")`, and `hello_ann()`
plays the role of `.greet()`. **A closure is an object with one method — minus the
class ceremony.** (Java's anonymous inner classes did the same trick: they copied
the `final` locals they used into the object. Python just does it automatically.)

#### Step 4 — *changing* a backpack variable needs `nonlocal`

Everything so far only **reads** from the backpack, and reading needs no special
syntax. The `make_counter` drill needs one thing more: updating the variable between
calls.

```python
def make_counter():
    count = 0
    def counter():
        nonlocal count             # "count is the one in my backpack — not a new variable"
        count = count + 1
        return count
    return counter

c = make_counter()
c()                                # -> 1
c()                                # -> 2
d = make_counter()
d()                                # -> 1    separate backpack, separate count
```

Why is the `nonlocal` line needed at all? Because of a Python rule you haven't
needed until now: **assigning to a name inside a function creates a local variable
of that function.** Python has no `int count;` declarations — the assignment itself
is the declaration. So without `nonlocal`, the line `count = count + 1` would mean
"create a brand-new `count` that belongs to `counter`" — and then reading
`count + 1` before that new variable has a value is an error:

```python
def make_counter_broken():
    count = 0
    def counter():
        count = count + 1          # forgot nonlocal -> Python: "count is a NEW local"
        return count
    return counter

make_counter_broken()()   # UnboundLocalError: cannot access local variable 'count'
                          # where it is not associated with a value
```

`nonlocal count` switches that rule off for one name: "don't create a local —
`count` means the variable in my backpack." Rule of thumb:

- **Read** a backpack variable → nothing special needed (`make_greeter`).
- **Assign to** a backpack variable → declare `nonlocal` first (`make_counter`).
- **See `UnboundLocalError` in a nested function** → you forgot `nonlocal`.

Calibration against Java: Java lambdas may only capture locals that are *effectively
final* — never reassigned; the compiler rejects anything else. So what `nonlocal`
permits is something Java flatly bans — that's why the Java column of this lesson
needed the `AtomicInteger` smuggling trick.

#### Why closures are worth the trouble

- **Function factories** — `make_multiplier(k)`, `make_adder(n)`: the standard
  phone-screen question for this topic, two lines each.
- **Decorators** (section 6) are closures — `memoize`'s cache dict lives in a
  backpack and survives between calls, exactly like `count`.
- **Robotics callbacks**: `on_frame(handler)` — a closure handler remembers its
  camera id or threshold with no one-off class in sight.

*(Optional curiosity, skip freely: the backpack is real and inspectable. After
`c = make_counter()`, the expression `c.__closure__[0].cell_contents` shows the
current `count`. Never needed in real code — just proof there's no magic.)*

### 4. Default values and keyword arguments replace overloading

Python has **no method overloading**: one name, one function. Write a second `def`
with the same name and it silently *replaces* the first. Optional behavior comes from
**default parameter values** instead:

```java
// Java: two overloads
void moveTo(double x, double y) { moveTo(x, y, 1.0); }
void moveTo(double x, double y, double speed) { ... }
```

```python
def move_to(x, y, speed=1.0): ...

move_to(1, 2)                 # speed = 1.0
move_to(1, 2, speed=0.5)      # a KEYWORD argument: named at the call site
move_to(y=2, x=1)             # keywords may come in any order
```

A **keyword argument** is an argument passed by parameter name (`speed=0.5`) rather
than by position. Java has nothing like it; it is why Python APIs with six optional
parameters stay readable. In real code (and this repo's tests) you will constantly
see calls like `sorted(xs, key=len, reverse=True)`.

Now the most famous bug in Python. To see it coming, ask a question you have
probably never asked: **when does the `speed=1.0` part actually run?**

Remember from the closure section that a `def` line is itself a statement that
Python *executes* — once, when it first reaches it (usually while loading the file)
— and that executing a `def` creates the function object. Here is the new fact: **at
that same moment, Python also computes every default value, once, and stores the
result on the function object.** A call that omits the argument is simply handed the
stored object. Not a fresh one — *the* stored one, every time.

For `speed=1.0` you will never notice, because a float is **immutable** — it cannot
be changed in place, and sharing a value nobody can change is harmless. But make the
default a *list* — which CAN be changed in place — and watch:

```python
def append_to(item, target=[]):    # the [] runs ONCE, here, at def time
    target.append(item)
    return target

append_to(1)                  # -> [1]      target = the one stored list
append_to(2)                  # -> [1, 2]   target = the SAME stored list. Surprise.
```

Timeline, spelled out:

```
file loads:      def line runs → ONE empty list is created and stored on append_to
append_to(1):    no target given → Python hands over the stored list → [1]
append_to(2):    no target given → the SAME list again              → [1, 2]
```

In Java terms: you expected `new ArrayList<>()` inside each call, but what you wrote
behaves like a **static field** initialized once at class-load time. The fix is the
**None-sentinel idiom** — a **sentinel** is a placeholder value meaning "the caller
gave nothing"; `None` (Python's `null`) is the standard choice, and the real object
gets built *inside the body*, which runs fresh on every call:

```python
def append_to(item, target=None):
    if target is None:
        target = []           # a FRESH list, created per call
    target.append(item)
    return target

append_to(1)                  # -> [1]
append_to(2)                  # -> [2]     independent calls, as you expected
```

Actionable rule: numbers, strings, `True`/`False`/`None` are safe as defaults
(immutable); never put `[]`, `{}`, `set()`, or any other changeable object in a
parameter default. (This is Gotcha 1 in `../LEARNING_POINTS.md`, and a favorite
interview trivia question: "what does this print, and why?")

#### The loop gotcha — functions made in a loop all see the LAST value

You now hold both ingredients (closures read their backpack *when called*; defaults
are computed *at def time*), so you can digest a classic trap. Build three tiny
functions in a loop:

```python
fns = [lambda: i for i in range(3)]     # three lambdas, made while i = 0, 1, 2
[f() for f in fns]                      # -> [2, 2, 2]    ...not [0, 1, 2]?!
```

Why: a backpack does **not** hold a photocopy of `i`'s value at packing time — all
three lambdas' backpacks contain the *same shared variable* `i`, and the loop keeps
overwriting it: 0, then 1, then 2. The lambda bodies run only *later*, and each one
then looks in the backpack and reads `i` **as it is now**: 2. One sentence to
remember: **a closure remembers where the variable lives, not what it was.**

Fix 1 — the interview classic, using the default-value rule you just learned.
Defaults are computed at def time, and here "def time" is *during the loop
iteration*, while `i` is still 0 (then 1, then 2):

```python
fns = [lambda i=i: i for i in range(3)]
[f() for f in fns]                      # -> [0, 1, 2]
```

Read `lambda i=i: i` as two different `i`s: the right one is the loop variable,
evaluated on the spot; the left one is a brand-new parameter belonging to this one
lambda, with that number frozen in as its default. A private photocopy per lambda,
instead of the shared variable.

Fix 2 — cleaner and more readable: a factory function. Each *call* of the outer
function packs a fresh backpack (that is exactly `make_greeter`):

```python
def make_fn(i):
    return lambda: i                    # captures THIS call's i

fns = [make_fn(i) for i in range(3)]
[f() for f in fns]                      # -> [0, 1, 2]
```

The real-world symptom of this bug is always the same: "all my callbacks act on the
last camera / button / task." When you see that, think *shared variable in the
backpack* — and reach for a factory or the `i=i` freeze. (Deep dive: Gotcha 8 in
[`../LEARNING_POINTS.md`](../LEARNING_POINTS.md).)

### 5. `*args` and `**kwargs` — variadic in both directions

**Variadic** means "accepts a variable number of arguments." Java has `Object...
varargs` for extra positional arguments; Python has that *plus* the keyword side:

```python
def describe_call(*args, **kwargs):
    # args   = a tuple of the extra positional arguments (a tuple is an immutable list)
    # kwargs = a dict of the extra keyword arguments, in the order they were passed
    return f"args={args} kwargs={kwargs}"

describe_call(1, 2, x=3)      # -> "args=(1, 2) kwargs={'x': 3}"
describe_call(7)              # -> "args=(7,) kwargs={}"   note (7,): a 1-element tuple
```

The same stars work at the *call site* to unpack: `f(*a_list)` spreads a list into
positional arguments, `f(**a_dict)` spreads a dict into keyword arguments. The
forwarding pattern `def wrapper(*args, **kwargs): return f(*args, **kwargs)` —
"accept anything, pass it through untouched" — is the backbone of every decorator
and of half the ML libraries you will use (`plt.plot(..., **style)`,
`Model(**config)`).

### 6. Decorators — functions that wrap functions

A **decorator** is a function that takes a function and returns a replacement for it
— usually a wrapper that adds behavior (caching, logging, timing, retries) around
the original. The `@name` line above a `def` is pure **syntactic sugar** (a shorthand
that changes nothing about semantics) for reassignment:

```python
@memoize
def slow(x): ...
# ...is EXACTLY equivalent to:
def slow(x): ...
slow = memoize(slow)
```

Java's closest relatives are a caching/logging proxy or Spring's annotation-driven
AOP — but there the framework does the weaving; here it is five lines of plain code:

```python
import functools

def memoize(f):
    cache = {}                            # captured by the closure below
    @functools.wraps(f)                   # copies f.__name__, f.__doc__ onto wrapper
    def wrapper(*args):
        if args not in cache:             # a tuple is hashable, so it can be a dict key
            cache[args] = f(*args)
        return cache[args]
    return wrapper
```

**Memoization** = caching a function's return values by its arguments so repeated
calls do no recomputation. The gotcha: without `@functools.wraps(f)`, the decorated
function reports `slow.__name__ == "wrapper"` and loses its docstring — which
wrecks logs, debuggers, and test output. Rule: every wrapper you write gets
`@functools.wraps`. And in production code you don't hand-roll this at all — the
standard library ships it as `functools.lru_cache`:

```python
from functools import lru_cache

@lru_cache(maxsize=None)      # in 3.9+, @functools.cache is the same thing
def fib(n):
    return n if n < 2 else fib(n - 1) + fib(n - 2)
fib(30)                       # -> 832040, in 31 real calls instead of ~2.7 million
```

### 7. EAFP — try first, apologize later

Java style is **LBYL** — "Look Before You Leap": pre-check every hazard
(`if (b != 0)`, `if (map.containsKey(k))`). Idiomatic Python is **EAFP** — "Easier to
Ask Forgiveness than Permission": attempt the operation and catch the specific
exception if it fails. Python exceptions are all *unchecked* (no `throws` clauses, no
compiler forcing you to catch), and raising/catching is cheap enough to use as a
normal control path:

```java
double safeDivide(double a, double b, double dflt) {
    if (b == 0) return dflt;          // LBYL pre-check
    return a / b;
}
```

```python
def safe_divide(a, b, default=None):
    try:
        return a / b                  # just try it
    except ZeroDivisionError:         # catch the NARROWEST exception that applies
        return default

safe_divide(10, 4)            # -> 2.5   (a single / always gives a float — no int truncation)
safe_divide(1, 0)             # -> None
safe_divide(1, 0, default=-1.0)   # -> -1.0
```

Two rules to act on: catch the most specific exception class you can (a bare
`except:` also swallows Ctrl-C and typos), and when re-raising a stored exception,
`raise last_exc` keeps the original **traceback** — Python's stack trace — intact,
so the log still points at the real failure line. See Gotcha 11 in
`../LEARNING_POINTS.md`.

### 8. Sort keys — a tuple is a Comparator chain

Python sorting never uses pairwise `compare(a, b)` logic. You supply a **key
function**: it maps each element to a value, and elements are ordered by their
mapped values. For multi-field ordering the key returns a tuple, because tuples
compare **lexicographically** — element by element, left to right, like dictionary
alphabetical order — which does exactly what
`Comparator.comparing(...).thenComparing(...)` does:

```java
tasks.sort(Comparator.comparing(Task::priority).thenComparing(Task::name));
```

```python
sorted(rows, key=lambda r: (r["priority"], r["name"]))   # priority, ties broken by name
(1, "build") < (1, "test") < (2, "deploy")               # -> True: how tuple ordering works
sorted(words, key=len, reverse=True)                     # descending: reverse=True
sorted(words, key=lambda w: (-counts[w], w))             # count DESC, then word ASC: negate the number
min(points, key=lambda p: p[0]**2 + p[1]**2)             # min/max take key= too
```

Two gotchas, spelled out. First: `sorted(xs)` returns a **new** list and leaves the
input untouched; `xs.sort()` sorts **in place** and returns `None` — so if you write
`result = xs.sort()` you have stored `None` and lost nothing but your sanity. Second:
both are **stable sorts** — elements that compare equal keep their original relative
order — which is why sorting by one field then another (or using a tuple key) is
reliable.

## Muscle memory

Type these until they require no thought:

```python
f = some_function; f(x)                            # store a function, call it — no .apply()
return lambda x: x * k                             # closure factory
nonlocal count                                     # assign to a backpack (outer-function) variable
def f(item, target=None):                          # the None-sentinel default
    if target is None: target = []
def wrapper(*args, **kwargs): return f(*args, **kwargs)   # accept-anything forwarding
@functools.wraps(f)                                # on every wrapper, always
try: ...
except ZeroDivisionError: ...                      # EAFP, narrowest exception
sorted(xs, key=lambda r: (r["a"], r["b"]))         # tuple key = thenComparing
xs.sort(key=len, reverse=True)                     # in place, returns None
```

## The drills

Work through `starter.py` top to bottom; each function's docstring restates the contract.

### `apply_n_times(f, n, x)`

Apply `f` to `x`, `n` times: `f(f(...f(x)))`; `n == 0` returns `x` unchanged.

```python
apply_n_times(lambda x: x * 2, 3, 1)   # -> 8
apply_n_times(str.upper, 1, "hi")      # -> "HI"   (methods are values too)
```

Where you'll see it: the function-as-argument shape underlies every callback API —
ROS subscriber callbacks, PyTorch hooks, `map`/`filter`. Interviewers rarely name
this drill, but it is the warm-up move inside simulation problems ("step this state
machine k times") and any "apply k operations to an array" prompt; fumbling `f(x)`
vs `f` here costs you the first two minutes of every such question.

### `make_multiplier(k)`

Return a function that multiplies its argument by `k` — a closure factory.

```python
triple = make_multiplier(3)
triple(5)                     # -> 15
make_multiplier(0.5)(10)      # -> 5.0   each factory call captures its own k
```

Where you'll see it: the classic closure screener ("write `make_adder(n)`") appears
verbatim in Python and JavaScript phone screens. In real work it is how you build
parameterized transforms: a scaling function per joint, a learning-rate schedule
`make_schedule(base_lr)`, a per-camera undistortion function.

### `make_counter()`

Return a function that yields 1, 2, 3, ... on successive calls; each counter independent.

```python
c = make_counter()
c(); c(); c()                 # -> 1, 2, 3
make_counter()()              # -> 1   fresh state per factory call
```

Where you'll see it: the standard `nonlocal` probe — "implement a counter without a
class" — and the setup for rate limiters, ID generators, and call-counting test spies
(this folder's own tests use one). If you write `count += 1` without `nonlocal` and
can't explain the UnboundLocalError, that is the signal the interviewer was fishing for.

### `safe_divide(a, b, default=None)`

Return `a / b`, or `default` when the division fails.

```python
safe_divide(10, 4)            # -> 2.5
safe_divide(1, 0, default=-1.0)   # -> -1.0
```

Where you'll see it: EAFP + default-argument style is judged in any code you write,
not as a standalone question. Division-by-zero guards show up constantly in robotics
math: normalizing a zero-length vector, success-rate and frames-per-second
computations, inverse-kinematics denominators. The reviewer wants
`try/except ZeroDivisionError`, not `if b == 0`.

### `append_to(item, target=None)`

Append `item` to `target`, creating a fresh list per call when `target` is omitted.

```python
append_to("x")                # -> ["x"]
append_to("y")                # -> ["y"]   NOT ["x", "y"] — no shared default list
```

Where you'll see it: the mutable-default trap is the single most-asked Python trivia
question in interviews ("what does this print and why?"), and a real bug generator in
ML code — `def train(model, callbacks=[])` quietly shares one callbacks list across
every training run in the process. The None-sentinel fix must be reflex.

### `describe_call(*args, **kwargs)`

Return the exact string `"args=(1, 2) kwargs={'x': 3}"` for `describe_call(1, 2, x=3)`.

```python
describe_call(1, 2, x=3)      # -> "args=(1, 2) kwargs={'x': 3}"
describe_call(7)              # -> "args=(7,) kwargs={}"
```

Where you'll see it: `*args, **kwargs` forwarding is the prerequisite for "write a
decorator" (a top-tier Python interview prompt — logging, timing, retry decorators)
and for reading any ML library signature: `torch.nn.Module.__init__(**kwargs)`,
`plt.plot(*args, **kwargs)`. If the star syntax slows you down, decorators are
unwritable.

### `compose(*funcs)`

Compose right-to-left: `compose(f, g, h)(x) == f(g(h(x)))`; empty compose is the
**identity function** (returns its input unchanged).

```python
add_one, double = (lambda x: x + 1), (lambda x: x * 2)
compose(add_one, double)(5)   # -> 11   double first, then add_one
compose()(99)                 # -> 99
```

Where you'll see it: "Function Composition" is a named LeetCode problem, and the
pipeline idea is everywhere in ML preprocessing — `torchvision.transforms.Compose`
chains image transforms exactly this way (though it applies them left-to-right, in
list order). Robotics inference stacks are compose chains in disguise: undistort →
resize → normalize → tensorize.

### `memoize(f)`

Decorator: cache `f`'s results by positional arguments; preserve `f`'s metadata with
`functools.wraps`.

```python
@memoize
def slow_add(a, b): return a + b
slow_add(1, 2); slow_add(1, 2)    # second call: cache hit, f runs once
slow_add.__name__                  # -> "slow_add", thanks to functools.wraps
```

Where you'll see it: memoization is the beating heart of top-down dynamic
programming — "Climbing Stairs", "House Robber", "Word Break", "Longest Increasing
Subsequence" all fall to `@lru_cache` on a recursive helper — and "LRU Cache" itself
is a famous design question one level up. In real work you slap `@lru_cache` on
expensive **pure** functions — ones whose result depends only on their arguments
and that change nothing outside themselves, so a cached answer is always safe to
reuse: forward-kinematics lookups, config parsing, tokenizer loading.

### `call_with_retry(f, attempts)`

Call `f()` up to `attempts` times; return the first success, or re-raise the last
exception; `attempts < 1` raises ValueError.

```python
call_with_retry(flaky_read, 3)    # retries up to 3 times
# all attempts failed -> the LAST exception propagates, traceback intact (bare `raise last_exc`)
```

Where you'll see it: "implement retry (with backoff)" is a staple practical screen
for infrastructure and robotics-systems roles — it tests EAFP, loop design, and
exception re-raising in one go. In the lab it is daily bread: serial reads from
the arm that drop bytes, camera frames that time out, gRPC calls to an inference
server that hiccups.

### `sort_by(records, *fields)`

Return a new list of dicts sorted ascending by the given field names, earlier fields
more significant; do not mutate the input.

```python
sort_by(rows, "priority", "name")
# -> sorted by priority, ties broken by name; `rows` itself unchanged
```

Where you'll see it: multi-key sorting via tuple keys is tested by name in "Top K
Frequent Elements" (count descending, word ascending — negate the count in the key),
"Merge Intervals" and "Non-overlapping Intervals" (sort by start), "Group Anagrams"
(sorted string as the grouping key), and "K Closest Points to Origin" (numeric key).
In robotics/ML you use it to rank detections by confidence, order candidate grasps
by score-then-distance, and sort experiment runs by (loss, timestamp).

## How to practice

```bash
# Against the reference solutions (should pass out of the box):
uv run pytest ramp_up/python/05_functions_and_closures -v

# Against YOUR implementation in starter.py:
PRACTICE=1 uv run pytest ramp_up/python/05_functions_and_closures -v
```

Deep dives referenced above — mutable defaults (Gotcha 1), late-binding closures
(Gotcha 8), EAFP (Gotcha 11), and the Java-to-Python container cheatsheet — live in
[`../LEARNING_POINTS.md`](../LEARNING_POINTS.md).
