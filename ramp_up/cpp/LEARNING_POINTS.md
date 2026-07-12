# A Java Developer's Map to C++

Dense reference card. Every section: the Java idiom you know, then what C++ actually does.
C++17 assumed throughout (that's what the exercises compile with).

---

## 1. Value semantics vs reference semantics — the #1 shift

In Java, every non-primitive variable is a reference; assignment and parameter passing
share the object. In C++, **objects are values by default**: assignment and parameter
passing **copy the whole object** unless you explicitly ask for a reference or pointer.

```java
// Java: one list, two names
List<Integer> a = new ArrayList<>(List.of(1, 2));
List<Integer> b = a;
b.add(3);            // a also sees [1, 2, 3]
void f(List<Integer> xs) { xs.add(9); }  // mutates caller's list
```

```cpp
// C++: assignment CLONES
std::vector<int> a = {1, 2};
std::vector<int> b = a;   // deep copy — independent vector
b.push_back(3);           // a is still {1, 2}

void f(std::vector<int> xs);        // pass by value: caller gets a COPY, xs mutations invisible
void f(std::vector<int>& xs);       // pass by reference: like Java — mutations visible
void f(const std::vector<int>& xs); // read-only view, no copy — the DEFAULT for inputs
```

Rules of thumb:
- Function inputs you only read: `const T&` (cheap, no copy, can't mutate).
- Small trivially-copyable types (`int`, `double`, `char`, small structs): plain `T` by value.
- Inputs you'd copy anyway (e.g. to sort your own version): take `T` by value and mutate the parameter.
- Outputs: just `return` the object — copies are elided / moved (see §10), so this is cheap.

`==` on containers/strings compares **contents** (like Java `.equals()`), not identity.
There is no Java-style "reference equality" unless you compare pointers.

## 2. Stack vs heap, and no GC

Java: primitives on the stack, every object on the heap, GC reclaims them eventually.
C++: **you choose**, and destruction is deterministic.

```cpp
void demo() {
    std::vector<int> v(1000);          // v itself on the stack; its 1000 ints on the heap,
                                       // managed BY the vector
    auto p = std::make_unique<Widget>(); // explicitly heap-allocated, owned by p
}   // <- both destroyed exactly here, in reverse declaration order. No GC, no pause, no maybe.
```

- Prefer stack/member objects (`std::string name;`) over heap (`new`). Java had no such choice.
- You should essentially never write `new`/`delete` by hand — containers and smart pointers own heap memory for you.
- Destruction order is deterministic: end of scope, reverse order of construction. Java's
  `finalize()` ("maybe, someday, on some thread") has no analogue because none is needed.

## 3. RAII — the replacement for try-with-resources / finally

RAII = Resource Acquisition Is Initialization: a constructor acquires the resource, the
**destructor** releases it, and the destructor runs deterministically at scope exit —
including when an exception unwinds the stack.

```java
// Java
try (var in = new FileInputStream(path)) {
    use(in);
}                       // close() called by try-with-resources
```

```cpp
// C++
{
    std::ifstream in(path);   // ctor opens
    use(in);
}                             // dtor closes — ALWAYS, even on exception
```

Everything uses this one mechanism: files (`fstream`), locks (`std::lock_guard`),
memory (`unique_ptr`, containers), timers, log scopes. Where Java needed a language
feature (`finally`, try-with-resources, `Cleaner`), C++ needs only destructors.
Exercise `02_ownership_and_raii` builds one from scratch.

## 4. Raw pointers vs references vs smart pointers

Java has exactly one kind of indirection (the nullable reference). C++ splits it into four,
by **ownership** and **nullability**:

| C++ type | Owns the object? | Nullable? | Java analogue | Use for |
|---|---|---|---|---|
| `T&` / `const T&` | no | never (must bind at init) | non-null reference param | function parameters, aliases |
| `T*` raw pointer | **no** (modern rule) | yes | nullable reference | optional non-owning access |
| `std::unique_ptr<T>` | yes, **sole** owner | yes | the *only* reference to an object | default for heap objects |
| `std::shared_ptr<T>` | yes, ref-counted | yes | a Java reference (shared, auto-freed) | genuinely shared ownership (rarer than you think) |

```cpp
auto u = std::make_unique<Robot>("arm");   // sole owner; freed when u dies
auto s = std::make_shared<Robot>("arm");   // ref-counted; freed when LAST shared_ptr dies
Robot* raw = u.get();                      // borrow: observe, never delete
Robot& ref = *u;                           // borrow with guaranteed non-null
```

- `unique_ptr` copies are **forbidden** at compile time — transfer with `std::move` (§10).
- `shared_ptr` behaves most like a Java reference, but don't default to it: the ref-count
  costs atomics, and cycles leak (use `weak_ptr` to break them — Java's GC handled cycles for free; ref-counting doesn't).
- A function taking `T*` or `T&` is *borrowing*; taking `unique_ptr<T>` by value is *consuming* (taking ownership).

## 5. const correctness — no Java equivalent

Java `final` only stops rebinding the variable. C++ `const` makes the **object itself**
read-only, and it's enforced transitively through the type system.

```java
final List<Integer> xs = ...;
xs.add(1);   // fine — final didn't protect the contents
```

```cpp
const std::vector<int> xs = {1, 2};
xs.push_back(3);                      // COMPILE ERROR

void print(const Robot& r) {
    r.name();      // OK — only if name() is declared const
    r.rename("x"); // COMPILE ERROR — non-const method on const ref
}

class Robot {
    std::string name() const;   // "const method": promises not to mutate *this
    void rename(std::string n); // non-const: may mutate
};
```

Habit to build: mark every method that doesn't mutate as `const`, and every read-only
parameter as `const T&`. Interviewers notice.

## 6. Headers, translation units, and the compile+link model

Java: one `.java` → one `.class`, imports resolved by the JVM/classpath.
C++: `.h`/`.hpp` **declarations** are textually `#include`d into each `.cpp`;
each `.cpp` compiles independently to a `.o`; the **linker** stitches them together.

```cpp
// robot.h — declarations (the "interface")
#pragma once                 // don't include me twice into one file
int maxTorque(int jointId);

// robot.cpp — definitions
#include "robot.h"
int maxTorque(int jointId) { return jointId * 10; }

// main.cpp
#include "robot.h"           // compiler now knows the signature
int main() { return maxTorque(3); }
```

```sh
c++ -std=c++17 -c robot.cpp -c main.cpp && c++ robot.o main.o -o app
```

- "undefined symbol" at the end = **linker** error (declaration seen, definition never linked) — the error Java never showed you.
- One Definition Rule: define a function in a header without `inline` and include it twice → linker error.
- Templates (§9) must live entirely in headers — the compiler generates code per instantiation.
- These exercises dodge all of this with single-file programs; real codebases don't.

## 7. Container rosetta table

| Java | C++ | Notes |
|---|---|---|
| `ArrayList<T>` | `std::vector<T>` | THE default container. Contiguous, cache-friendly. `push_back`, `size`, `v[i]` (unchecked) / `v.at(i)` (throws) |
| `String` | `std::string` | **Mutable** value type. `+=` appends in place. Compare with `==` (contents). `s.substr(i, len)` — note *length*, not end index |
| `HashMap<K,V>` | `std::unordered_map<K,V>` | `m[k]` **inserts a default-constructed value if absent** (great for counters, surprising for lookups). Absence-safe lookups: `m.find(k)`, `m.count(k)`, `m.at(k)` (throws) |
| `TreeMap<K,V>` | `std::map<K,V>` | Red-black tree, keys iterate sorted, O(log n) |
| `HashSet<T>` | `std::unordered_set<T>` | `s.insert(x).second` is `true` if newly inserted — replaces Java's `add()` boolean |
| `TreeSet<T>` | `std::set<T>` | sorted, O(log n) |
| `ArrayDeque<T>` | `std::deque<T>` | also the backing store of `std::queue`/`std::stack` adaptors |
| `PriorityQueue<T>` | `std::priority_queue<T>` | **max**-heap by default — Java's is a min-heap! Min-heap: `priority_queue<T, vector<T>, greater<T>>` |
| `int[]` / fixed array | `std::array<T, N>` | size in the type, stack-allocated |
| `Optional<T>` | `std::optional<T>` | value type, no allocation |
| `long` | `long long` (or `int64_t`) | C++ `long` is 32-bit on Windows, 64-bit on Linux/Mac. Want Java's `long`? Say `long long` |

Algorithms live in `<algorithm>`, not on the containers:
`std::sort(v.begin(), v.end())`, `std::reverse(...)`, `std::find(...)`,
`std::max_element(...)` — Collections/Arrays utilities, generalized via iterators.

## 8. Iterators vs `Iterator`

Java's `Iterator` is an object with `hasNext()/next()`. A C++ iterator is a
**generalized pointer**: `*it` dereferences, `++it` advances, and a range is a *pair*
of iterators `[begin, end)` where `end` points one past the last element.

```cpp
std::vector<int> v = {3, 1, 4};
for (auto it = v.begin(); it != v.end(); ++it) { use(*it); }
auto hit = std::find(v.begin(), v.end(), 4);
if (hit != v.end()) { ... }          // "not found" == end(), not null

// The idiom you'll actually write 95% of the time — range-based for:
for (int x : v)              { ... }   // copies each element (fine for ints)
for (const auto& s : names)  { ... }   // read-only, no copies — the default for objects
for (auto& s : names)        { s += "!"; }  // mutate in place

// Maps yield pairs; C++17 structured bindings replace Map.Entry:
for (const auto& [key, value] : myMap) { ... }   // vs entry.getKey()/getValue()
```

**Iterator invalidation** (no Java analogue — Java throws `ConcurrentModificationException`;
C++ gives you undefined behavior, §11): growing a `vector` may reallocate and dangle every
iterator/reference/pointer into it. Don't hold iterators across `push_back`/`erase`.

## 9. Templates vs generics — no type erasure

Java generics erase to `Object` + casts; one `ArrayList` bytecode serves all `T`, so
primitives need boxing. C++ templates **generate separate code per type at compile time**.

```java
List<Integer> xs;         // erased: really List<Object>; int is boxed
```

```cpp
template <typename T>
T maxOf(const T& a, const T& b) { return b < a ? a : b; }

maxOf(3, 5);          // compiler STAMPS OUT maxOf<int> — real ints, no boxing
maxOf(std::string("a"), std::string("b"));  // and a separate maxOf<std::string>
```

Consequences:
- `vector<int>` stores raw ints contiguously — this is why C++ wins at numeric/robotics workloads.
- No `T extends Comparable<T>` bounds (pre-C++20): constraints are structural. If `T` lacks
  `operator<`, you get an error at the *usage inside the template* — hence C++'s famously long error messages.
- Template definitions must be visible at instantiation → they live in headers (§6).

## 10. Move semantics and `std::move` — practical version

Copying a big vector is expensive, so C++ lets an object's guts be **stolen** when the
source won't be used again. Java never needed this (copying a reference is free).

```cpp
std::vector<int> big = makeBig();
std::vector<int> a = big;             // COPY: both alive, expensive
std::vector<int> b = std::move(big);  // MOVE: b steals the heap array; big is now empty-ish
```

What you actually need to know:
- `std::move` moves nothing — it's a *cast* meaning "I'm done with this; you may pillage it."
- After moving from a variable, treat it as unusable (assign or destroy only).
  Exception: a moved-from `unique_ptr` is *guaranteed* null.
- `return localVector;` is already optimal — the compiler elides or moves. **Never** write `return std::move(local);`.
- The place you *must* write it: transferring a `unique_ptr` (copy is deleted):

```cpp
std::unique_ptr<Buffer> p = makeBuffer(64);
consume(std::move(p));   // ownership handed off; p is now nullptr
```

## 11. Undefined behavior — the concept Java never taught you

Java errors are *defined*: out-of-bounds throws, null access throws, races have JMM semantics.
In C++, breaking certain rules gives **undefined behavior**: the standard permits *anything* —
crash, garbage, silently "working" today and failing in the demo.

Classic UB you'll meet:
- `v[i]` out of bounds (use `v.at(i)` to throw instead while learning)
- dereferencing null/dangling pointers; using an iterator after invalidation (§8)
- signed integer overflow (`INT_MAX + 1` is UB, not wraparound!)
- reading an uninitialized local: `int x; use(x);` — locals are NOT zeroed like Java fields
- use-after-free, double-free (largely prevented by smart pointers/containers)

Defenses: compile with `-Wall -Wextra`, test with sanitizers
(`-fsanitize=address,undefined`), and let RAII types own everything. Key mindset shift:
**a C++ program that runs green may still be wrong** — Java taught you the runtime would
catch you; here nothing promises to.

## 12. `auto` — type inference at the declaration

Like Java's `var`, plus deduction subtleties Java doesn't have:

```cpp
auto n = v.size();                  // std::size_t (unsigned!) — beware `n - 1` when n == 0
auto it = m.find(k);                // spares you std::unordered_map<...>::iterator
auto s = getName();                 // a COPY (auto never deduces a reference)
auto& r = getName();                // a reference — you must write the &
const auto& [k, v] = *m.begin();    // structured bindings, const reference
```

Use `auto` where the type is obvious or hideous (iterators); write the type where it
carries information. And remember: `auto` alone copies — `auto&` / `const auto&` to alias.

---

## Cheat sheet: "In Java I would... → in C++ I should..."

| In Java you'd... | In C++ you should... |
|---|---|
| pass an object to a method (shared by default) | pass `const T&` (read) or `T&` (mutate) — plain `T` copies |
| `new Foo()` and let GC clean up | stack object, or `std::make_unique<Foo>()` |
| try-with-resources / `finally` | RAII — a scope `{}` and a destructor |
| `map.getOrDefault(k, 0) + 1` then `put` | `++m[k]` (operator[] auto-inserts zero) |
| `set.add(x)` returning boolean | `s.insert(x).second` |
| `Collections.sort(list)` | `std::sort(v.begin(), v.end())` |
| `for (Map.Entry<K,V> e : m.entrySet())` | `for (const auto& [k, v] : m)` |
| `a.equals(b)` on collections/strings | `a == b` (deep by default) |
| `long` for 64-bit math | `long long` (plain `long` is platform-dependent) |
| hand the only reference to someone | `std::move` a `unique_ptr` into them |
| trust the runtime to throw on mistakes | assume nothing: UB is silent — sanitize, `-Wall`, RAII |
