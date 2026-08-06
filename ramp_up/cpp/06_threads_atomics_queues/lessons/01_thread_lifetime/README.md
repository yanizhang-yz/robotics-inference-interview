# 06.1 — Thread lifetime

After this lesson you can identify who owns each `std::thread`, where it starts,
and the join boundary that makes its borrowed data safe.

## The problem this lesson solves

A camera or inference stage often benefits from parallel work, but a C++ thread
starts during construction and remains *joinable* until it is joined or its
ownership is transferred. Destroying a joinable `std::thread` calls
`std::terminate`. Work cannot safely outlive stack variables captured by reference.

## The lesson

`parallel_sum(std::span<const int>)` splits one borrowed range into disjoint halves.
Each worker writes its own partial sum, so the writes do not race. The calling thread
joins both workers before reading the partials or returning past the span's lifetime.

```cpp
std::thread first([&] { /* sum the left half into left */ });
std::thread second([&] { /* sum the right half into right */ });
first.join();
second.join();
return left + right;
```

Construction starts the work. `join()` waits for it and ends joinability. Scheduling
order is deliberately irrelevant: the result depends only on disjoint ranges and the
join-before-read relationship.

## How interviewers test this

**Prediction:** predict joinability and destruction behavior.

**Implementation:** implement `parallel_sum` with owned join boundaries.

**Follow-up:** explain exception-safe alternatives such as `jthread`.

**Evidence:** account for worker lifetime and every join.

Expect to point to every thread's owner and every exit path. Explain why forgetting a
join terminates the process, why detaching would make the captured span unsafe, and
why two independent partial sums need no mutex.

## Muscle memory

For each thread you create, answer: what data does it borrow, who owns the thread
object, and where is it joined? Read shared results only after the relevant join.

## The drills

Implement `parallel_sum`. Split at `size() / 2`, launch two workers, join both, and
combine the partials. The assertions cover empty, positive, and negative-containing
ranges.

## How to practice

```bash
PRACTICE=1 uv run pytest ramp_up/cpp/06_threads_atomics_queues/lessons/01_thread_lifetime -q
uv run pytest ramp_up/cpp/06_threads_atomics_queues/lessons/01_thread_lifetime -q
```

Continue to the [next lesson](../02_lambda_captures/).
