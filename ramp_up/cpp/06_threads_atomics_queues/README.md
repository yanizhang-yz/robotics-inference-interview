# 06 — Threads, Atomics, and the Producer/Consumer Queue

After this lesson you will be able to: launch a thread and reliably wait for it,
explain and *demonstrate* a data race, fix one two different ways (a mutex and an
atomic) and say when each is right, put a thread to sleep until there is work for it,
and assemble the class interviewers ask for by name — a **bounded blocking queue**.
That queue is also the single most important pattern of a robotics inference job: a
camera thread producing frames, an inference thread consuming them, and the queue
between them keeping the robot honest when the model can't keep up. Every term is
defined at first use; every snippet's behavior was verified with
`clang++ -std=c++20 -Wall -Wextra -Werror=return-type -pthread` on this Mac.

## The toolbox you already own

You have already written concurrent code — and not toy code. In
[`questions/05_systems/001_thread_safe_ring_buffer`](../../../questions/05_systems/001_thread_safe_ring_buffer/)
you built a fixed-capacity buffer that multiple threads could push to and pop from,
and you made it safe the standard Python way: one `threading.Lock`, acquired by every
public method with `with self._lock:`. This lesson is that same toolbox with the
training wheels off. Here is the whole lesson in the Python you speak:

```python
import threading, queue

# Launching and waiting:
t = threading.Thread(target=work)   # a thread object; nothing runs yet
t.start()                           # NOW it runs
t.join()                            # wait for it to finish

# The shared-counter bug, and fix 1 — your ring buffer's lock:
counter += 1                        # unlocked shared state: a bug (yes, even in Python)
with lock:                          # threading.Lock: one thread inside at a time
    counter += 1                    # safe

# Sleeping until there's work — with the famous while-loop rule:
with cond:                          # threading.Condition
    while not items:                # while, NOT if — wakeups can be spurious
        cond.wait()                 # releases the lock while sleeping
    consume(items.popleft())

# And the class that packages all of the above:
q = queue.Queue(maxsize=4)          # bounded, blocking FIFO
q.put(frame)                        # blocks while the queue is FULL
frame = q.get()                     # blocks while the queue is EMPTY
```

Every line above has a direct C++ counterpart, most of them one-to-one — and the last
one, Python hands you finished. Here you will build it yourself from a mutex and two
condition variables, which is exactly what interviewers ask. It is also exactly how
CPython builds it: open `queue.py` in the standard library and you will find a
`collections.deque`, one `Lock`, and two `Condition`s named `not_empty` and
`not_full`. By the end of this lesson you will have written that file in C++.

And remember the ring-buffer question's follow-up #1 — "make `pop` *block* until an
item arrives"? That follow-up is this lesson's centerpiece.

## What C++ takes away: the GIL

One thing before the syntax, because it changes how careful you must be.

CPython has a **Global Interpreter Lock** (the GIL): one interpreter-wide lock that a
thread must hold to execute Python bytecode. Only one thread runs Python at a time.
Your Python threads *interleaved* — the interpreter let each run for a slice
(`sys.getswitchinterval()` says 5 ms by default) and then offered the lock to another —
but two of them never truly executed on two cores at the same instant. That is why
Python threads never sped up your compute, and why NumPy drops to C (and the GIL is
released) for the heavy loops.

The GIL also quietly *masked* bugs. `counter += 1` is several bytecodes — read, add,
write — and a thread can lose the interpreter between them, so the unlocked version
really is broken; the ring-buffer README's follow-up #3 said it straight. But the
switch points are a few bytecodes wide and a few milliseconds apart, so racy Python
usually gets lucky. Measured on this Mac: two Python threads each doing
`counter += 1` 100,000 times with no lock returned exactly 200000, three runs out of
three. Broken code, perfect score. The GIL never made your code thread-safe; it made
your bugs shy.

C++ has no GIL. Threads genuinely execute at the same instant on different cores.
That is the reward — the camera thread and the inference thread really do run in
parallel, which is the entire reason an inference engineer writes C++ — and it has a
price: the same increment experiment in C++ returned 111271, 110246, then 104515.
The bug your Python papered over corrupts data here, visibly, almost every run.
True parallelism is the reward; discipline is the price. This lesson is the
discipline.

## The lesson

### 1. `std::thread` — construction starts it; you must ALWAYS join

A **thread** is an independent line of execution inside your process: it gets its own
call stack, but shares the same memory as every other thread — that sharing is where
all the trouble in this lesson comes from. Launch-and-wait is nearly a
transliteration of the Python you know:

```python
t = threading.Thread(target=work)     # Python: construct...
t.start()                             # ...then start
t.join()                              # wait for it
```

```cpp
std::thread t(work);                  // C++: constructing IS starting
t.join();                             // wait for it
```

Two differences worth naming. There is **no `start()`** — constructing a
`std::thread` launches it immediately. And where Python's `target=` takes any
callable, C++ takes any callable too — in practice a lambda:

```cpp
#include <thread>

std::thread t([] { std::cout << "hello from the new thread\n"; });
std::cout << "hello from main\n";
t.join();   // wait here until the new thread finishes
// Output: both lines, then main continues past join().
// On this Mac, "hello from main" consistently printed first across a dozen runs —
// starting an OS thread takes a moment — but BOTH orders are legal. Never build
// logic on scheduling order.
```

Data goes into the thread by lambda capture (lesson 01's rules: capture by value for
a copy; capture by reference **only** when you are certain the thread joins before
the variable dies — a thread outliving a stack variable it references is a dangling
reference, lesson 02's crash).

Now the rule Python never had. In Python, forgetting `join()` is forgiven — the
program runs on, and the interpreter even waits for non-daemon threads at exit. In
C++, letting a `std::thread` object die while its thread is still "joinable"
(running, or finished but never joined) **terminates your whole program**:

```cpp
{
    std::thread t([] { std::cout << "hello from thread\n"; });
}   // t destroyed without join()
// Verified: the process dies with "libc++abi: terminating", exit code 134.
// Not an exception you can catch — std::terminate() aborts the program.
```

So the discipline is mechanical: **every thread you create, you join** — including on
early-return paths. (There is a fire-and-forget escape hatch, `t.detach()` — the
distant cousin of Python's daemon threads. In robotics code treat it as a bug: a
detached thread can touch freed memory during shutdown.)

Launching N threads uses a familiar friend: `std::thread` cannot be copied (two
owners of one OS thread — same logic as `unique_ptr` in lesson 02) but can be moved,
so a vector of threads works naturally:

```cpp
std::vector<std::thread> pool;
for (int t = 0; t < 4; ++t) {
    pool.emplace_back([] { /* work */ });   // construct the thread in place
}
for (auto& th : pool) {
    th.join();                              // join EVERY one
}
```

### 2. The data race — the bug the GIL was hiding

Names first. **Shared state** is any memory two or more threads can touch. A **data
race** is two threads touching the same memory at the same time, at least one of them
writing, with nothing coordinating them. In C++ a data race is **undefined
behavior** — lesson 02's term: the standard permits *anything*, from wrong numbers to
crashes. (CPython is gentler: the GIL is held for the duration of each bytecode, so
memory itself never tears — races there garble your logic at the seams *between*
bytecodes. C++ softens nothing.)

Why is one-line `++counter` a race at all? For exactly the reason `counter += 1` was
several bytecodes: it is not one step. The CPU can only do arithmetic in a
**register** — a private scratch cell inside the processor — so every `++` is three
steps: **read** the value from memory into a register, **add** one, **write** it
back. Two threads can interleave those steps:

```text
counter is 5
Thread A: read 5            Thread B: read 5      <- both read BEFORE either writes
Thread A: add  -> 6         Thread B: add  -> 6
Thread A: write 6           Thread B: write 6     <- two increments, counter is 6, not 7
```

One increment silently vanished. Watch it happen for real:

```cpp
int counter = 0;                       // shared, protected by nothing
auto work = [&counter] {
    for (int i = 0; i < 100000; ++i) {
        ++counter;                     // read-modify-write: three interruptible steps
    }
};
std::thread t1(work);
std::thread t2(work);
t1.join();
t2.join();
std::cout << counter << "\n";
// Expected: 200000. One run gave 143412; the next gave 100185; then 103309.
// The value is wrong AND different almost every run — that run-to-run
// inconsistency is the signature of a data race. It can also pass by luck,
// which is why "it worked when I tested it" means nothing for threaded code.
```

Hold the two experiments side by side: Python, 200000 three times out of three; C++,
a different wrong number nearly every run. Same bug in both programs — only C++ shows
it to you, because only C++ actually ran the threads at the same time.

This is the `racy_increment_demo` drill. Note what our test does with it: it
**prints** the value and asserts nothing — undefined behavior has no result you may
rely on, not even a reliably wrong one.

### 3. Fix 1 — `std::mutex` + `std::lock_guard` (this is `with lock:`)

A **mutex** ("mutual exclusion") is a lock that only one thread can hold at a time;
any other thread that asks for it is put to sleep until the holder releases it. You
know it as `threading.Lock` — your ring buffer had exactly one, covering every
method. The code that runs while holding the mutex is called a **critical section** —
a stretch only one thread can be inside at once.

In Python you learned never to call `lock.acquire()` and `lock.release()` by hand —
you write `with lock:` so the release happens automatically, even when the body
raises. C++ has no `with` statement, and needs none, because lesson 02 already built
the machine: RAII. A destructor is an `__exit__` the language runs for every object
at every scope exit. `std::lock_guard` locks the mutex in its constructor and unlocks
in its destructor — **the guard's enclosing scope IS the with-block**:

```python
with lock:                  # acquire
    counter += 1            # critical section
# release — even on exception
```

```cpp
{
    std::lock_guard<std::mutex> lock(m);   // constructor acquires
    ++counter;                             // critical section
}                                          // destructor releases — even on exception
```

Applied to the racy counter:

```cpp
int counter = 0;
std::mutex m;
auto work = [&] {
    for (int i = 0; i < 100000; ++i) {
        std::lock_guard<std::mutex> lock(m);   // constructor takes the lock
        ++counter;                             // critical section: one thread at a time
    }                                          // destructor releases it, exception or not
};
std::thread t1(work), t2(work);
t1.join(); t2.join();
// counter -> 200000. Verified: exact, every run (three for three).
```

Never call `m.lock()` and `m.unlock()` by hand — the same advice as
acquire/release, with sharper teeth: one early `return` or exception between them and
the mutex stays locked forever, and every thread that later wants it sleeps for good.
RAII exists so that bug cannot be written.

While we are here, the other famous way to sleep forever: a **deadlock** is two
threads each holding a mutex the other one is waiting for — neither can ever proceed.
(Python with two `Lock`s deadlocks identically; the GIL does not save you, because
the threads are asleep in the locks, not fighting over the interpreter.) You cannot
deadlock with the single mutex above; the classic recipe needs two mutexes taken in
opposite orders by two threads. The interview answer: always acquire multiple mutexes
in one agreed global order (or use `std::scoped_lock`, which locks a set of mutexes
deadlock-free for you).

### 4. Fix 2 — `std::atomic<int>`: the tool Python never grew

Here the Python map runs out. Python has no everyday atomic-counter type — under the
GIL a simple counter was mostly-safe by accident, so the language never needed one;
Pythonistas just reach for a `Lock`. C++ threads truly run in parallel, so C++ names
the hardware directly.

An operation is **atomic** if it happens as one indivisible step — no thread can
observe it half-done or wedge between its halves. Modern CPUs have instructions that
do read-modify-write in one indivisible step, and `std::atomic<int>` makes `++` use
them. No mutex, nobody sleeps:

```cpp
std::atomic<int> counter{0};
auto work = [&] {
    for (int i = 0; i < 100000; ++i) {
        ++counter;                    // one indivisible hardware instruction
    }
};
std::thread t1(work), t2(work);
t1.join(); t2.join();
// counter.load() -> 200000. Verified: exact, every run.
// .load() reads the value atomically; .store(x) writes it atomically.
```

So when do you use which?

- **Atomic**: ONE variable, updated independently. Frame counters, dropped-frame
  counters, and the classic shutdown flag — `std::atomic<bool> running{true};` that
  a control loop checks and a signal handler flips.
- **Mutex**: an invariant spanning **more than one variable**. Your ring buffer is
  the perfect example: slots, `head`, and `size` had to change *together*, which is
  precisely why its one `Lock` wrapped each whole method instead of each field.
  Making each variable individually atomic does not make their *combination*
  consistent: another thread could see `size` incremented before the slot is
  written. Multi-variable invariant ⇒ one mutex around the whole update.

### 5. `condition_variable` — `threading.Condition`, same rules

Now aim at the queue. A consumer thread finds the queue empty — what does it do? The
naive answer is a **busy-wait**: loop, checking over and over. That burns an entire
CPU core doing nothing, which on a robot is stolen from the model you are trying to
run. The right answer you have already met in Python: `threading.Condition`, and
C++'s `std::condition_variable` is the same machine — a *waiting room attached to a
mutex*. A thread that finds nothing to do goes to sleep in the waiting room —
**releasing the mutex while it sleeps** — and a thread that changes the state rings
the bell to wake it.

Two mechanics to nail down:

- **`unique_lock`, not `lock_guard`.** `wait()` must unlock the mutex while sleeping
  (otherwise no producer could ever get in to add work) and re-lock it before the
  sleeper wakes up — Python's `cond.wait()` does exactly the same dance under its
  `with cond:` block. `std::lock_guard` cannot unlock mid-scope; its flexible
  sibling `std::unique_lock` can — that is the entire reason `wait()` demands one.
- **The while-loop rule survives, verbatim.** A **spurious wakeup** is a thread
  waking with *nobody having notified it* — the operating system is explicitly
  allowed to do this. It is why the Python docs make you write
  `while not predicate: cond.wait()` instead of an `if`. C++ folds that loop into an
  overload: `cv.wait(lock, predicate)` — a **predicate** is just a function
  returning bool, "is there work for me?" — behaves exactly like
  `while (!predicate()) cv.wait(lock);`. Python grew the identical shortcut,
  `cond.wait_for(predicate)`. In both languages: always use the predicate form; it
  re-checks the condition on every wakeup, spurious or real.

```cpp
std::mutex m;
std::condition_variable cv;
std::deque<int> box;                      // std::deque: a FIFO — collections.deque

std::thread consumer([&] {
    std::unique_lock<std::mutex> lock(m);
    cv.wait(lock, [&] { return !box.empty(); });  // sleeps, mutex RELEASED, until true
    std::cout << "got " << box.front() << "\n";   // woke up: mutex re-held, work exists
});

std::this_thread::sleep_for(std::chrono::milliseconds(50));  // consumer sleeps meanwhile
{
    std::lock_guard<std::mutex> lock(m);
    box.push_back(42);                    // change the state under the mutex...
}
cv.notify_one();                          // ...then ring the bell (Python: cond.notify())
consumer.join();
// Output: got 42        (verified; notify_all() = Python's cond.notify_all())
```

### 6. Assemble it: `BoundedQueue<T>` — your own `queue.Queue(maxsize=)`

A **bounded queue** is a FIFO with a maximum size fixed at construction — the
interface of `queue.Queue(maxsize=4)`: `put` blocks while full, `get` blocks while
empty. The bound is not a nicety — it is the safety mechanism, and you already
derived why in the ring-buffer question. Run the numbers for a real robot: one
1280×720 RGB frame is 1280·720·3 ≈ 2.6 MB. Camera produces at 30 fps; suppose the
model only manages 20 fps. An *unbounded* queue then grows by 10 frames — ~26 MB —
every second, about 1.5 GB per minute, until the process dies. Worse than the memory
is the **latency**: the frame at the head of that swelling queue is seconds old, so
the robot plans against the past. A bounded queue caps both at once: when it fills,
`push()` puts the *producer* to sleep until the consumer catches up. That
slower-stage-slows-the-faster-stage effect is called **backpressure**. (The other
industrial option is dropping the oldest frame instead of blocking — that is
literally your ring buffer's `push_overwrite`; see the drill's "Where you'll see
it".)

Design, from the rules you now own:

- Storage plus size is a multi-variable invariant ⇒ **one mutex** guards everything
  (§4), taken by `push`, `pop`, and `size`.
- Two reasons to sleep ⇒ **two condition variables**: `not_full_` (where full-queue
  `push`ers wait) and `not_empty_` (where empty-queue `pop`pers wait). One shared
  condition variable *can* work, but then a push might wake another pusher —
  a thread that still cannot proceed. Two waiting rooms wake exactly the threads
  that can make progress. This is not our invention: CPython's `queue.Queue` is
  built from precisely this pair — one `Lock`, `not_empty`, `not_full` (plus a third
  condition for `Queue.join()`'s task tracking), storage in a `collections.deque`.
  You are re-implementing `queue.py`, minus the interpreter.

Here is `push()`, line by line — building `pop()` as its mirror image is your drill:

```cpp
void push(T value) {
    std::unique_lock<std::mutex> lock(mutex_);                    // 1
    not_full_.wait(lock, [this] { return items_.size() < capacity_; });  // 2
    items_.push_back(std::move(value));                           // 3
    not_empty_.notify_one();                                      // 4
}
```

1. Take the mutex — all queue state lives behind it.
2. Sleep until there is room. The predicate re-checks after every wakeup (§5), and
   the mutex is released while sleeping, so consumers can get in and make room.
3. There is room and we hold the lock: append. `std::move` because a frame is big —
   hand it over, don't copy it (lesson 03).
4. If a consumer is asleep in `not_empty_`, there is now an item for it: ring that
   bell. (Notify the *other* side's waiting room — pushes wake poppers.)

`pop()` is the same shape with the roles swapped: wait on `not_empty_` until
`!items_.empty()`, take `items_.front()`, `pop_front()`, notify `not_full_`, return
the value.

One last C++ word: `size()` should be `const` (lesson 02's const-correctness), but
even a read must lock the mutex, and locking mutates the mutex object. The keyword
**`mutable`** on a field means "this field may change even inside `const` methods" —
`mutable std::mutex mutex_;` is the standard idiom, and the compiler enforces the
rest.

Rosetta stone for what you just built:

| Your `BoundedQueue<T>` | Python `queue.Queue` |
|---|---|
| `BoundedQueue<int> q(4);` | `queue.Queue(maxsize=4)` |
| `q.push(x)` — blocks when full | `q.put(x)` |
| `q.pop()` — blocks when empty | `q.get()` |
| `q.size()` | `q.qsize()` |

### 7. The floor below this one (recognize the words, don't go there yet)

Everything above used locks, or atomics with their default guarantees — that default
has a name, `std::memory_order_seq_cst` ("sequentially consistent": all threads
agree on one global order of atomic operations), and it is the right default. Below
it lies **lock-free programming**: building structures like this queue with no mutex
at all, using atomics with *relaxed* memory orders (`std::memory_order` is the knob)
and compare-and-swap loops — where hazards like the **ABA problem** live (a value
changes from A to B and back to A, so a compare-and-swap concludes "nothing
happened" and corrupts the structure). You have brushed against this floor once
already: the ring-buffer question's follow-up #2 — "single-producer/single-consumer
without a lock" — lives here, and the questions track has it waiting as
`questions/05_systems/005_lock_free_spsc_queue_cpp`. Senior interview loops
name-drop these to gauge depth; at this stage the winning answer is knowing the
words, saying "I'd reach for a proven lock-free queue library before writing one",
and being solid on everything above this paragraph.

## Muscle memory

Type these until they require no thought:

```cpp
std::thread t([&] { work(); });   t.join();        // Thread(target=work).start() + .join()
std::vector<std::thread> pool;                     // N threads: emplace_back, then
for (auto& th : pool) th.join();                   //   join every one
std::mutex m;
{ std::lock_guard<std::mutex> lock(m); /* ... */ } // `with lock:` — the braces ARE the block
std::atomic<int> n{0};   ++n;   n.load();          // no Python analog; hardware-indivisible
std::atomic<bool> running{true};                   // the shutdown flag idiom
std::unique_lock<std::mutex> lock(m);              // wait() needs unique_lock...
cv.wait(lock, [&] { return ready; });              // ...and ALWAYS the predicate form
cv.notify_one();                                   // wake one; notify_all() wakes all
mutable std::mutex mutex_;                         // so const methods can lock
```

## The drills

Open `starter.cpp`; each stub restates its own hints. Runtimes are small on purpose —
the whole binary finishes in well under a second.

### `racy_increment_demo()`

Two threads, each `++counter` 100000 times on a plain shared `int`, both joined;
return the (almost certainly wrong) count.

```cpp
racy_increment_demo()   // one run gave 104776; another 103268; correct would be 200000
// main() PRINTS this value and asserts nothing — run the binary a few times and
// watch it change. A data race has no result you may rely on, even a wrong one.
```

Where you'll see it: "what is a data race?" / "spot the bug in this code" is the
standard concurrency opener, and the three-step story of `++` (read, modify, write)
is the expected answer — plus one sentence you can now say from experience: "Python's
GIL usually hides this exact bug; C++ exposes it." In robotics code the racy `int`
is usually the latest pose, a frames-processed counter, or an e-stop flag shared
between a sensor callback thread and the control loop — the bug arrives exactly this
innocently.

### `safe_count_mutex(threads, iters)`

Same counting, now correct: `threads` workers each add `iters` increments under a
`std::mutex` + `std::lock_guard`; return the exact total.

```cpp
safe_count_mutex(4, 25000)   // -> 100000, every single run
safe_count_mutex(1, 1000)    // -> 1000: one thread must work too
```

Where you'll see it: "make this code thread-safe" is the follow-up to the opener,
and saying "RAII guard, not manual lock/unlock — it releases on exceptions, like a
`with` block" is what separates C++ answers from C answers. Real version: any robot
state touched by more than one thread — the current joint targets, the latest camera
intrinsics, a shared map — sits behind exactly this mutex-plus-guard pattern, the
same shape as your ring buffer's `with self._lock:`.

### `safe_count_atomic(threads, iters)`

Same contract, no mutex: a `std::atomic<int>` counter.

```cpp
safe_count_atomic(4, 25000)   // -> 100000, every single run — and faster: nobody sleeps
```

Where you'll see it: the immediate interview follow-up — "do you even need a lock
here?" — and the "atomic vs mutex, when?" question (one independent variable:
atomic; multi-variable invariant: mutex). In real systems these are your telemetry
counters (frames in, frames dropped, inferences done) and the
`std::atomic<bool> running` shutdown flag every control loop checks.

### `BoundedQueue<int>`

The centerpiece: capacity-constructor, blocking `push`, blocking `pop`, locked
`size` — built from one mutex and the two condition variables `not_full_` /
`not_empty_` (all four fields are already declared in the starter; you write the
three method bodies). `main()` checks it three ways:

```cpp
BoundedQueue<int> q(4);              // 1. Single-threaded FIFO sanity:
q.push(1); q.push(2); q.push(3);     //    pops come back 1, 2, 3 — first in, first out
                                     // 2. Producer/consumer: a thread pushes 0..999
                                     //    (the "camera") while main pops and sums (the
                                     //    "inference loop"); sum must be exactly 499500.
                                     //    Capacity 8 << 1000 items forces both sides
                                     //    to block and wake many times.
                                     // 3. Backpressure: into a capacity-2 queue, a
                                     //    producer attempts THREE pushes; main waits,
                                     //    then asserts size() == 2 and that push #3 has
                                     //    not completed — then pops once to release it.
```

A word on test #3, since you'll read it in `main()`: it waits (with a capped poll)
for the first two pushes to land, then grants a 200 ms grace window. That makes it
"deterministic enough" rather than formally deterministic — 200 ms is thousands of
times longer than a push takes, so if your `push` *could* wrongly complete, it would
have, and the asserts catch it. The stubs in the starter are deliberately
non-blocking no-ops so a wrong implementation fails an assert fast instead of
hanging the test (the pytest harness allows 30 s; nothing here should ever near it).

Where you'll see it: **"implement a thread-safe (blocking) queue" is a named staple
of C++ and systems interviews** — this drill *is* that question, condition variables
and all, and the follow-ups are §5–6 verbatim: "why `while`/predicate?", "why two
condition variables?", "what if the producer is faster — and that's the word
*backpressure*". In robotics inference work this queue is the job: the camera thread
pushes frames at 30 fps, the inference thread pops at whatever the model manages,
and the bound decides your latency and memory. ROS 2 exposes the same idea as
subscription queue depth in its QoS settings; production perception stacks often
choose the drop-oldest variant instead (a robot wants the *newest* frame, stale ones
are worthless) — which is your Python ring buffer's `push_overwrite` policy. Once
your blocking version works, a `try_push` that evicts the front is a ten-line remix
worth attempting.

## How to practice

```bash
# Against the reference solutions (should pass out of the box):
uv run pytest ramp_up/cpp/06_threads_atomics_queues -v

# Against YOUR implementation in starter.cpp:
PRACTICE=1 uv run pytest ramp_up/cpp/06_threads_atomics_queues -v
```

Or compile and run directly — `main()` asserts every drill and prints `ALL TESTS
PASSED`:

```sh
clang++ -std=c++20 -Wall -Wextra -Werror=return-type -pthread -o /tmp/threads starter.cpp && /tmp/threads
```

Note the flag this lesson adds: `-pthread` tells the compiler and linker to build
with POSIX threads support. Linux `g++` requires it for `std::thread`; Apple clang
accepts it harmlessly (verified warning-free here), and the pytest harness passes it
for you. Run the solution binary a few times in a row and watch the racy count
change while every assert stays green — that contrast is the whole lesson.

## The road ahead

Everything in this lesson is CPU-side plumbing for the GPU work it feeds — and the
concepts transfer almost one-to-one.

- **A CUDA stream is a producer/consumer queue.** A *stream* is a queue of GPU work:
  the CPU pushes kernel launches and memory copies onto it, and the GPU consumes
  them in order. You are the producer, the GPU is the consumer, and
  `cudaStreamSynchronize()` is `join()` — wait here until the worker finishes.
  Launching a kernel is like constructing a `std::thread`: it starts *now*, and the
  bugs come from forgetting who is still running.
- **The camera→inference pipeline you built is the real architecture.** Production
  inference is a chain of stages — capture → preprocess → infer → postprocess — each
  a thread (or pool), connected by exactly the bounded queues of §6, with
  backpressure deciding end-to-end latency. The GPU adds one more overlap trick:
  while frame N computes, frame N+1's bytes upload on a second stream (double
  buffering) — the same "keep both sides busy, bound the backlog" instinct.
- **Atomics scale to thousands of threads.** CUDA's `atomicAdd()` is §4's hardware
  indivisible read-modify-write with 10,000 threads hammering the counter instead of
  two — histograms and reductions live on it, and "when do atomics serialize your
  kernel" is a GPU interview classic.
- **Where the track picks this up:** the questions track continues this exact lesson
  as `questions/05_systems/003_bounded_blocking_queue_cpp` (this drill grown into a
  full interview question), `005_lock_free_spsc_queue_cpp` (§7's floor below), and
  `006_thread_pool_inference_server_cpp` — this queue plus a pool of workers, which
  is a miniature inference server. The queue you built today is the load-bearing
  wall of all three.
