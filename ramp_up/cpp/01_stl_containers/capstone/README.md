# Capstone — Telemetry Triage

## Card

This is the real game. No blanks to fill: you write a whole program in
`starter.cpp`, stage by stage. A robot streams joint telemetry as text
lines on stdin:

```
arm_shoulder 0.42 36.5
```

That is `<joint> <position> <temperature>`. Your program takes the stage
number as its only argument (`./capstone 1 < telemetry.txt`) and prints
that stage's report. Pass a stage and the next one unlocks. Everything
from lessons 01–10 gets used: values, references, vectors, algorithms —
plus `std::string` parsing, `std::map`, and `std::set`, which are yours to
meet here.

## Stage 1 — Parse the stream

Read every line from `std::cin`. A line is **valid** when it has exactly
three whitespace-separated fields: a joint name, then two numbers. Any
other line — too few fields, extra fields, unparseable numbers — is
**skipped**, never fatal. (`std::istringstream` turns one line into
fields.)

Print exactly one line:

```
parsed=N skipped=M
```

## Stage 2 — Clamp and group

Safety first: every position must live in `[-1.57, 1.57]`. Clamp each
valid sample's position into that range (`std::clamp`), counting the
samples you had to clamp, per joint. Group samples by joint —
`std::map<std::string, ...>` keeps keys sorted for free — and for each
joint print one line, joints in ascending name order, numbers with exactly
two decimals (`std::fixed` + `std::setprecision(2)`):

```
arm_elbow max_pos=1.57 clamped=1
```

`max_pos` is the largest absolute position for that joint **after**
clamping.

## Stage 3 — Triage report

The maintenance summary. Count the distinct joints (`std::set`, or your
map's keys). Compute each joint's **mean temperature**, then find the two
hottest joints — highest mean first, ties broken by name ascending. Print
exactly one line (one joint total means one name, no comma):

```
joints=4 hottest=arm_elbow,base_yaw
```

## Takeaway

- You just wrote a complete, testable C++ program: parse, transform,
  aggregate, report — the skeleton of every telemetry and inference tool.
- Ownership stayed obvious the whole way: values in vectors, groupings in
  maps, mutations through references, no `new` in sight.
- Module 02 (ownership and RAII) starts where this ends: what happens
  when a resource is not a value.

## Deep dive

Real robot fleets emit exactly this kind of stream, and real pipelines
survive on the discipline you just practiced: malformed input is counted
and skipped, never trusted and never fatal; safety limits are enforced
before data flows onward; reports are deterministic — sorted keys, fixed
precision — so two runs of the same log diff clean.

The staging mirrors how such tools actually grow. Parsing comes first
because nothing downstream is meaningful until the stream is structured.
Grouping and clamping come next, where `std::map` earns its place: it
keeps joints sorted while you accumulate per-joint state through
references into the map's values. The report stage is pure standard
library — `std::set` for distinct counts, `std::sort` with a custom
comparator for hottest-first with a name tie-break, and top-K selection
by taking the front of the sorted order.

If a stage will not go green, run your binary by hand against the fixture
(`./a.out 2 < fixtures/stage2_input.txt`) and compare with the matching
`stage2_expected.txt` — exact text comparison is the contract, including
the two-decimal formatting.
