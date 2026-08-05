# Capstone — Telemetry Triage

The module capstone: a complete program written from a near-empty
`starter.cpp`, in three graded stages. It integrates everything from
lessons 01–10 and introduces three tools best met inside a real program:
`std::string` parsing with `std::istringstream`, `std::map` for grouped
aggregation, and `std::set` for distinct counts.

A robot streams joint telemetry as text lines on stdin:

```
arm_shoulder 0.42 36.5
```

That is `<joint> <position> <temperature>`. Your program takes the stage
number as its only argument (`./capstone 1 < telemetry.txt`) and prints that
stage's report. Build the stages in order — each reuses the code of the one
before it.

## Stage 1 — Parse the stream

Read every line from `std::cin`. A line is **valid** when it has exactly
three whitespace-separated fields: a joint name, then two numbers. Any other
line — too few fields, extra fields, unparseable numbers — is **skipped**,
never fatal. (`std::istringstream` turns one line into fields; extraction
with `>>` reports success as a boolean, the same pattern as lesson 06's
"might be absent" thinking.)

Print exactly one line:

```
parsed=N skipped=M
```

## Stage 2 — Clamp and group

Safety first: every position must live in `[-1.57, 1.57]`. Clamp each valid
sample's position into that range (`std::clamp`, from lesson 01), counting
the samples you had to clamp, per joint. Group samples by joint —
`std::map<std::string, ...>` keeps keys sorted for free — and for each joint
print one line, joints in ascending name order, numbers with exactly two
decimals (`std::fixed` + `std::setprecision(2)`):

```
arm_elbow max_pos=1.57 clamped=1
```

`max_pos` is the largest absolute position for that joint **after** clamping
(lesson 04's `max_abs_position`, grown up).

## Stage 3 — Triage report

The maintenance summary. Count the distinct joints (`std::set`, or your
map's keys). Compute each joint's **mean temperature**, then find the two
hottest joints — highest mean first, ties broken by name ascending
(a tuple-style comparator, exactly like the Python ramp's multi-key sort).
Print exactly one line (one joint total means one name, no comma):

```
joints=4 hottest=arm_elbow,base_yaw
```

## Debugging a stage

Exact text comparison is the contract, including the two-decimal formatting.
When a stage fails, run your binary by hand against the fixture and diff:

```bash
clang++ -std=c++20 -Wall -Wextra -Werror=return-type starter.cpp -o capstone
./capstone 2 < fixtures/stage2_input.txt | diff - fixtures/stage2_expected.txt
```

Real pipelines survive on exactly the discipline this program practices:
malformed input is counted and skipped, never trusted and never fatal;
safety limits are enforced before data flows onward; reports are
deterministic — sorted keys, fixed precision — so two runs of the same log
diff clean.

## How to practice

```bash
PRACTICE=1 uv run pytest ramp_up/cpp/01_stl_containers/capstone -q
```

Drop the `PRACTICE=1` to run the tests against the reference `solution.cpp`.
When all three stages pass, continue to
[Module 02 — Ownership and RAII](../../02_ownership_and_raii/).
