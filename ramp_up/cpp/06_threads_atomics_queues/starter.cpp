// 06_threads_atomics_queues — YOUR attempt. Fill in the TODO bodies, then run:
//   PRACTICE=1 uv run pytest ramp_up/cpp/06_threads_atomics_queues -v
// Or compile and run directly (note the extra -pthread vs earlier lessons):
//   clang++ -std=c++17 -Wall -pthread -o /tmp/threads starter.cpp && /tmp/threads
// The stubs compile as-is but fail main()'s asserts until you implement them.
// The BoundedQueue stubs are deliberately non-blocking no-ops, so a wrong
// answer always fails an assert fast instead of hanging the test.
// You're done when the last line printed is: ALL TESTS PASSED

#include <atomic>
#include <cassert>
#include <chrono>
#include <condition_variable>
#include <cstddef>
#include <deque>
#include <iostream>
#include <mutex>
#include <thread>
#include <utility>
#include <vector>

// racy_increment_demo() -> the final value of an UNSYNCHRONIZED counter that two
// threads each ++ 100000 times. Correct answer: 200000; a data race makes the
// real result wrong and different almost every run. main() prints it and never
// asserts on it — the run-to-run inconsistency is the lesson.
// JAVA: the same bug: a plain int field, two threads running count++.
// C++:  capture `counter` by reference in a lambda, run it on two std::thread
//       objects, join BOTH, return the counter. No mutex, no atomic — the bug
//       is the point.
int racy_increment_demo() {
    // TODO: implement (see README §1–2: launch two threads, join both)
    return 0;
}

// safe_count_mutex(4, 25000) -> exactly 100000, every run.
// JAVA: synchronized (lock) { count++; }
// C++:  a plain int counter plus std::mutex; inside the loop take a
//       std::lock_guard<std::mutex> lock(m); then ++counter. Launch `threads`
//       workers with pool.emplace_back(lambda), then join every one.
int safe_count_mutex(int threads, int iters) {
    // TODO: implement
    (void)threads;
    (void)iters;
    return 0;
}

// safe_count_atomic(4, 25000) -> exactly 100000, every run — no mutex involved.
// JAVA: AtomicInteger count; count.incrementAndGet();
// C++:  std::atomic<int> counter{0}; ++counter is one indivisible hardware
//       instruction. Return counter.load() (= AtomicInteger.get()).
int safe_count_atomic(int threads, int iters) {
    // TODO: implement
    (void)threads;
    (void)iters;
    return 0;
}

// A bounded, blocking, thread-safe FIFO queue — Java's ArrayBlockingQueue<T>.
// push() blocks while the queue is full; pop() blocks while it is empty.
// This is the camera->inference handoff: camera thread push()es frames, the
// inference thread pop()s them; a full queue sleeps the camera (backpressure).
// The private fields you need are already declared at the bottom of the class.
template <typename T>
class BoundedQueue {
public:
    explicit BoundedQueue(std::size_t capacity) : capacity_(capacity) {
        (void)capacity_;  // delete me once capacity_ is really used (silences -Wunused-private-field)
    }

    // Blocks while the queue is full. JAVA: ArrayBlockingQueue.put(value).
    // C++:  std::unique_lock<std::mutex> lock(mutex_);  (unique_lock — wait() needs it)
    //       not_full_.wait(lock, [this] { return items_.size() < capacity_; });
    //       push_back the value (std::move it), then not_empty_.notify_one().
    void push(T value) {
        // TODO: implement
        (void)value;  // stub is a no-op ON PURPOSE: fail asserts fast, never hang
    }

    // Blocks while the queue is empty. JAVA: ArrayBlockingQueue.take().
    // C++:  mirror image of push(): wait on not_empty_ until !items_.empty(),
    //       take items_.front() (std::move it), pop_front(), then
    //       not_full_.notify_one(), and return the value.
    T pop() {
        // TODO: implement
        return T{};  // stub returns a dummy ON PURPOSE: fail asserts fast, never hang
    }

    // C++: lock mutex_ (a lock_guard is enough — no wait() here) and return
    //      items_.size(). mutex_ is `mutable` so this const method may lock it.
    std::size_t size() const {
        // TODO: implement
        return 0;
    }

private:
    std::size_t capacity_;
    mutable std::mutex mutex_;           // mutable: even const size() must lock it
    std::condition_variable not_full_;   // push() sleeps here when the queue is full
    std::condition_variable not_empty_;  // pop() sleeps here when the queue is empty
    std::deque<T> items_;                // plain FIFO storage — Java's ArrayDeque
};

int main() {
    // racy_increment_demo: printed, deliberately NOT asserted. Run the binary a
    // few times — the number changes run to run; that inconsistency IS the bug.
    {
        int racy = racy_increment_demo();
        std::cout << "racy count (correct answer 200000): " << racy << "\n";
    }

    // safe_count_mutex: exact, every run
    assert(safe_count_mutex(4, 25000) == 100000);
    assert(safe_count_mutex(1, 1000) == 1000);

    // safe_count_atomic: exact, every run
    assert(safe_count_atomic(4, 25000) == 100000);
    assert(safe_count_atomic(8, 10000) == 80000);

    // BoundedQueue: single-threaded FIFO basics first
    {
        BoundedQueue<int> q(4);
        assert(q.size() == 0);
        q.push(1);
        q.push(2);
        q.push(3);
        assert(q.size() == 3);
        assert(q.pop() == 1);  // first in, first out
        assert(q.pop() == 2);
        assert(q.pop() == 3);
        assert(q.size() == 0);
    }

    // BoundedQueue: a real producer/consumer handoff. The producer thread plays
    // the camera; this thread plays the inference loop. Capacity 8 is far
    // smaller than 1000 items, so both sides repeatedly block and wake.
    {
        BoundedQueue<int> q(8);
        std::thread producer([&q] {
            for (int i = 0; i < 1000; ++i) {
                q.push(i);
            }
        });
        long long sum = 0;
        for (int i = 0; i < 1000; ++i) {
            sum += q.pop();
        }
        producer.join();
        assert(sum == 499500);  // 0+1+...+999: every item arrived exactly once
        assert(q.size() == 0);
    }

    // BoundedQueue: push really BLOCKS when the queue is full (backpressure).
    // The producer tries capacity+1 pushes; push #3 must park it. Timing note:
    // this test is "deterministic enough" rather than formally deterministic —
    // the 200ms grace sleep is thousands of times longer than a push takes, so
    // if push #3 COULD complete, it would have, and the asserts would catch it.
    {
        BoundedQueue<int> q(2);
        std::atomic<int> pushed{0};
        std::thread producer([&] {
            for (int v : {10, 20, 30}) {  // one more push than the queue can hold
                q.push(v);
                ++pushed;
            }
        });

        // Wait for the first two pushes to land (capped poll, so a broken push
        // can't hang this test — the asserts below fail instead).
        for (int i = 0; i < 500 && pushed.load() < 2; ++i) {
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
        // Now give push #3 a generous window to (incorrectly) sneak through.
        std::this_thread::sleep_for(std::chrono::milliseconds(200));

        assert(pushed.load() == 2);  // producer is still stuck inside push #3
        assert(q.size() == 2);       // the queue never grew past its capacity
        assert(q.pop() == 10);       // make room -> push #3 wakes up and finishes
        producer.join();
        assert(q.size() == 2);
        assert(q.pop() == 20);
        assert(q.pop() == 30);
        assert(q.size() == 0);
    }

    std::cout << "ALL TESTS PASSED" << std::endl;
    return 0;
}
