# 09 — Introductory Templates

## Problem

A robot records timestamped values from several subsystems. Joint positions are
`double` values, while robot modes are `std::string` values. Build one
`TimedSample` wrapper and one `latest_sample` function that work for both
without copying the same code for every value type.

## Mental model

A template is a compile-time recipe. `TimedSample<T>` says, “make a timestamp
wrapper whose `value` has whatever concrete type replaces `T`.” When the
compiler sees `TimedSample<double>`, it creates a concrete timestamp-plus-
`double` type. When it sees `TimedSample<std::string>`, it creates a separate
timestamp-plus-string type.

Likewise, `latest_sample` is a function recipe. The compiler instantiates one
version for a `std::vector<TimedSample<double>>` and another for a
`std::vector<TimedSample<std::string>>`. This differs from a runtime “any
object” container: each instantiated version has a known, checked value type.

Template definitions must be visible at the point where the compiler
instantiates them. In practice, put a function template's full definition in a
header or in the same `.cpp` file that uses it; a declaration alone in a header
is usually not enough.

## Application

The same timestamp wrapper lets an inference or robotics pipeline represent a
joint reading and a mode update consistently. `latest_sample` returns a
non-owning pointer to the final element of an existing vector, or `nullptr`
when no sample exists. The vector remains the owner, so the returned pointer is
valid only while that vector stays alive and is not changed in a way that moves
its elements.

## Prediction checkpoints

Before running the program, answer these questions:

1. What concrete type is `joints`? What concrete type does
   `latest_sample(joints)` instantiate?
2. What concrete type is `modes`? Why is its instantiated `value` a string
   rather than a number?
3. Why does `latest_sample(joints)` need no runtime check such as “is this a
   double sample?”
4. What should `latest_sample` return for an empty
   `std::vector<TimedSample<int>>`, and why is that a useful result?
5. Why must the full template definition be visible where this program calls
   it?

## Guided implementation

In `starter.cpp`, keep the template struct exactly as written. Implement
`latest_sample` as a function template: if `samples.empty()` is true, return
`nullptr`; otherwise return the address of `samples.back()`. `back()` refers to
the newest element because this application stores samples in arrival order.

## Verification

Run the reference with:

```bash
.venv/bin/python -m pytest ramp_up/cpp/01_stl_containers/lessons/09_introductory_templates -q
```

Run the learner starter with:

```bash
PRACTICE=1 .venv/bin/python -m pytest ramp_up/cpp/01_stl_containers/lessons/09_introductory_templates -q
```

The shared runner compiles with C++20 and `-Wall -Wextra -Werror=return-type`. The
untouched starter is expected to fail at `latest_joint != nullptr`; it does not
dereference the null pointer or hang.

## Explain it

- `TimedSample<double>` and `TimedSample<std::string>` are different concrete
  types generated from one source-level recipe.
- The function's `T` is deduced from the vector argument, so callers normally
  do not write the type explicitly.
- Each instantiation only accepts matching sample and value types, so type
  safety is checked at compile time rather than through runtime tags.
- `latest_sample` borrows from the vector with a pointer; it does not copy the
  sample or take ownership.

## Next connection

Standard-library types such as `std::vector<T>` use the same template model.
As the pipeline gains richer records, templates let you write reusable storage
and utility code while retaining the concrete types that C++ can optimize and
check.
