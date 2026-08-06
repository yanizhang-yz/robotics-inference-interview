# 06.2 — Lambda captures across threads

After this lesson you can choose value and reference captures from their lifetime and
sharing semantics instead of relying on a broad default capture.

## The problem this lesson solves

Launching workers in a loop is easy to get subtly wrong. Capturing the loop index by
reference lets workers observe a later value, while capturing a short-lived local by
reference can leave a dangling access after its scope ends.

## The lesson

`offset_samples` allocates the final output size before launching workers. Each lambda
captures `i` and `offset` by value, while borrowing the input and output by reference:

```cpp
workers.emplace_back(
    [&, i, offset] { output[i] = samples[i] + offset; });
```

Every worker owns stable copies of the slot index and offset. Workers write different
`output[i]` elements, so those writes are disjoint. Joining all workers before return
keeps the referenced vectors alive and makes the completed ordered result visible.

## How interviewers test this

**Prediction:** predict copied, shared, and dangling captures.

**Implementation:** implement ordered parallel offsets.

**Follow-up:** explain why disjoint result slots avoid a mutex.

**Evidence:** name each captured object's owner and lifetime.

You may be shown `[&]` inside a loop and asked why every worker can use the same final
index. Name which captures are values, which are borrows, and the join boundary that
keeps each borrow valid. Do not promise a worker execution order; the slot mapping
provides stable output order.

## Muscle memory

Capture small per-task identifiers by value. Capture shared storage by reference only
when its lifetime extends through every join, and prove concurrent writes are to
disjoint locations or are synchronized.

## The drills

Implement `offset_samples(samples, offset)`, launch one worker per element, and join
every worker. The assertions compare against a hand-derived ordered vector and cover
empty input.

## How to practice

```bash
PRACTICE=1 uv run pytest ramp_up/cpp/06_threads_atomics_queues/lessons/02_lambda_captures -q
uv run pytest ramp_up/cpp/06_threads_atomics_queues/lessons/02_lambda_captures -q
```

Continue to the [next lesson](../03_data_races_and_mutexes/).
