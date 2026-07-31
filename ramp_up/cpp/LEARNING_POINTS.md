# C++20 Reference Card

## 1. Program structure, compilation, linking, headers, and translation units

Rule: declarations belong in headers; each `.cpp` is a translation unit, and every
definition needed by a program must be linked exactly once.

```cpp
int add(int a, int b) { return a + b; }
int main() { return add(2, 3) == 5 ? 0 : 1; }
```

Compile with `c++ -std=c++20 -Wall -Wextra -Wpedantic demo.cpp -o demo`.
Mistake: defining a non-`inline` function in a header included by multiple translation units.
Learn it in [Module 01](01_stl_containers/).

## 2. Values, references, pointers, and `const`

Rule: pass read-only objects as `const T&`; use `T&` for a required mutable alias and
`T*` only when absence is meaningful.

```cpp
#include <string>
std::size_t size_of(const std::string& text) { return text.size(); }
int main() { const std::string name = "lidar"; return size_of(name) == 5 ? 0 : 1; }
```

Mistake: accepting `const T&` and then trying to mutate the caller's value.
Learn it in [Module 01](01_stl_containers/).

## 3. Containers, algorithms, iterators, `std::optional`, and `std::span`

Rule: prefer contiguous `std::vector` for resizable sequences; algorithms consume
iterator ranges, and `std::span` borrows a contiguous range without owning it.

```cpp
#include <algorithm>
#include <span>
#include <vector>
int main() { std::vector<int> v{3, 1, 2}; std::sort(v.begin(), v.end());
             std::span<const int> view{v}; return view[0] == 1 ? 0 : 1; }
```

Mistake: retaining an iterator, pointer, or reference across a vector reallocation.
Learn it in [Module 01](01_stl_containers/).

## 4. Scope, lifetime, ownership, destructors, and RAII

Rule: a local object is destroyed at scope exit in reverse construction order; let an
owner's destructor release its resource.

```cpp
#include <memory>
int main() { auto value = std::make_unique<int>(7); return *value == 7 ? 0 : 1; }
```

Mistake: calling `delete` on a pointer borrowed from `unique_ptr::get()`.
Learn it in [Module 02](02_ownership_and_raii/).

## 5. Copies, moves, copy elision, Rule of Zero, and Rule of Five

Rule: prefer the Rule of Zero; return values directly, and use `std::move` only when
transferring a named resource that will not be used as before.

```cpp
#include <string>
std::string label() { return "frame"; }
int main() { std::string name = label(); return name == "frame" ? 0 : 1; }
```

Mistake: writing `return std::move(local);`, which can prevent copy elision.
Learn it in [Module 03](03_move_semantics_rule_of_five/).

## 6. Runtime polymorphism and virtual destruction

Rule: a base used polymorphically needs a virtual destructor; mark overridden methods
with `override`.

```cpp
#include <memory>
struct Sensor { virtual ~Sensor() = default; virtual int read() const = 0; };
struct Camera : Sensor { int read() const override { return 1; } };
int main() { std::unique_ptr<Sensor> sensor = std::make_unique<Camera>(); return sensor->read() == 1 ? 0 : 1; }
```

Mistake: deleting a derived object through a base pointer whose destructor is not virtual.
Learn it in [Module 04](04_virtual_functions_and_vtables/).

## 7. Object layout, allocation, cache locality, AoS, and SoA

Rule: objects in `std::vector<T>` are contiguous; choose AoS when processing records
together and SoA when scanning one field across many records.

```cpp
#include <vector>
struct Point { float x; float y; };
int main() { std::vector<Point> points{{1, 2}, {3, 4}}; return points[1].y == 4 ? 0 : 1; }
```

Mistake: assuming field sizes simply add up while ignoring alignment and padding.
Learn it in [Module 05](05_memory_layout_and_cache/).

## 8. Threads, mutexes, RAII locks, atomics, condition variables, and shutdown

Rule: protect compound shared-state operations with a mutex and an RAII lock; use an
atomic only for the independent state it actually represents.

```cpp
#include <atomic>
#include <thread>
int main() { std::atomic<int> ready{0}; std::thread t([&] { ready.store(1); });
             t.join(); return ready.load() == 1 ? 0 : 1; }
```

Mistake: waiting on a condition variable without a predicate that also handles shutdown.
Learn it in [Module 06](06_threads_atomics_queues/).

## 9. Undefined behavior, compiler warnings, and sanitizer commands

Rule: data races, dangling access, and out-of-bounds access are undefined behavior;
warnings and sanitizers are part of normal development.

```cpp
#include <array>
int main() { std::array<int, 1> values{4}; return values.at(0) == 4 ? 0 : 1; }
```

Use `c++ -std=c++20 -Wall -Wextra -Wpedantic demo.cpp` and
`c++ -std=c++20 -fsanitize=address,undefined -g demo.cpp -o demo`.
Mistake: treating a passing debug run as proof that undefined behavior is harmless.
Learn it in [Module 05](05_memory_layout_and_cache/).

## 10. Function-signature decision table

Rule: make ownership, mutation, nullability, and range extent visible in the signature.

| Type | Use when |
|---|---|
| `T` | The function needs its own value or the type is cheap to copy. |
| `const T&` | The function reads a non-null existing object without copying. |
| `T&` | The function must mutate a caller-owned, non-null object. |
| `T*` | The function borrows an object that may be absent. |
| `std::span<T>` | The function borrows a contiguous mutable range. |
| `std::unique_ptr<T>` | The function consumes or returns exclusive ownership. |

```cpp
#include <span>
int sum(std::span<const int> values) { int total = 0; for (int value : values) total += value; return total; }
int main() { int values[]{2, 3}; return sum(values) == 5 ? 0 : 1; }
```

Mistake: using a raw pointer to imply ownership or to hide the range length.
Learn it in [Module 02](02_ownership_and_raii/).
