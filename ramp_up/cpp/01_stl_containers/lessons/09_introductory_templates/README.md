# 09 — Introductory Templates

## Card

A template is a **compile-time recipe** for a family of concrete types or
functions.

```cpp
template <typename T>
struct TimedSample { std::int64_t timestamp_ns; T value; };
```

`TimedSample<double>` and `TimedSample<std::string>` are separate
concrete types from that one recipe. A function template works the same
way: the compiler deduces `T` from the argument and instantiates one
checked version per value type — no runtime type tags. The full
definition must be visible where it is instantiated.

## Predict

`latest_sample(joints)` runs with no runtime check that the samples hold
`double`s. Why is that safe?

- A) The compiler instantiated a version of `latest_sample` fixed to `TimedSample<double>`
- B) The vector stores a type tag that is checked when elements are accessed
- C) It is not safe — a wrong value type would surface as `nullptr` at runtime

<!-- predict
answer: A
why-A: Right — each instantiation is generated for one concrete value type, so a mismatch is a compile error, never a runtime case.
why-B: Templates add no runtime tags; that is the "any object" container model this design deliberately avoids.
why-C: `nullptr` means only "the vector is empty" — a type mismatch would fail to compile, never reach runtime.
-->

## Drill

In `starter.cpp`, keep the template struct exactly as written and
implement `latest_sample`: return `nullptr` when `samples.empty()`,
otherwise the address of `samples.back()` — the newest sample, since this
application stores samples in arrival order.

Manual check: `PRACTICE=1 uv run pytest ramp_up/cpp/01_stl_containers/lessons/09_introductory_templates -q`

## Takeaway

- `TimedSample<double>` and `TimedSample<std::string>` are different
  concrete types generated from one source-level recipe.
- `T` is deduced from the vector argument, and each instantiation is
  checked at compile time — no runtime type tags.
- `latest_sample` borrows with a pointer; the vector keeps ownership of
  the sample.

## Deep dive

A template is a compile-time recipe. `TimedSample<T>` says: make a
timestamp wrapper whose `value` has whatever concrete type replaces `T`.
When the compiler sees `TimedSample<double>` it generates a concrete
timestamp-plus-double type; `TimedSample<std::string>` generates a
separate timestamp-plus-string type. `latest_sample` is a function recipe
in the same way — one version is instantiated for a vector of double
samples and another for a vector of string samples, with `T` deduced from
the argument so callers normally never spell it out. This is the opposite
of a runtime "any object" container: each instantiation has a known,
checked value type.

That one wrapper lets a robotics or inference pipeline represent a joint
reading (`double`) and a mode update (`std::string`) consistently.
`latest_sample` returns a non-owning pointer to the final element —
`back()` is the newest because samples arrive in order — or `nullptr`
when no sample exists, a useful result for the empty case. The vector
remains the owner, so the returned pointer is valid only while that
vector stays alive and is not changed in a way that moves its elements.

One mechanical rule: template definitions must be visible at the point of
instantiation, so put the full definition in a header or in the same
`.cpp` file that uses it. `std::vector<T>` itself follows this model —
templates are how the standard library writes reusable storage and
utility code that C++ can still optimize and check.
