// DRILL BRIEF
// Concept: Integrate thread lifetime, mutex/atomic synchronization, and bounded
// producer/consumer backpressure in one camera-to-inference capstone.
// Scenario: Workers count completed inference jobs while a bounded FIFO carries
// camera samples to a slower consumer.
// Implement: Retain and complete racy_increment_demo, safe_count_mutex,
// safe_count_atomic, and BoundedQueue without changing the assertions.
// Behavior: Both safe counters are exact; the queue is FIFO, bounded, blocking,
// and delivers every item. The racy result is observed and printed but NEVER
// asserted because a data race is undefined behavior and need not manifest.
// Example: safe_count_mutex(4, 25000) returns 100000. Edge: the racy value is never asserted and a full capacity-2 queue blocks its third push.
// Interview focus: Identify thread ownership, shared invariants, mutex versus
// atomic scope, the not-full/not-empty predicates, and their notifications.
// Tests: Safe counts, FIFO delivery, once-only delivery, and producer backpressure.
// Run: PRACTICE=1 uv run pytest ramp_up/cpp/06_threads_atomics_queues/test_solution.py -q
// Done when: The final output line is ALL TESTS PASSED.
//
//   PRACTICE=1 uv run pytest ramp_up/cpp/06_threads_atomics_queues/test_solution.py -v
// Or compile and run directly (note the extra -pthread vs earlier lessons):
//   clang++ -std=c++20 -Wall -Wextra -Werror=return-type -pthread -o /tmp/threads starter.cpp && /tmp/threads
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

// Return one deliberately unsynchronized observation for comparison only.
int racy_increment_demo() {
    // TODO: implement the documented two-worker experiment.
    return 0;
}

// Return threads*iterations exactly using a mutex-protected counter invariant.
int safe_count_mutex(int threads, int iters) {
    // TODO: implement
    (void)threads;
    (void)iters;
    return 0;
}

// Return threads*iterations exactly using one atomic counter transition.
int safe_count_atomic(int threads, int iters) {
    // TODO: implement
    (void)threads;
    (void)iters;
    return 0;
}

// BoundedQueue must preserve FIFO, never exceed capacity, and block at full or empty.
template <typename T>
class BoundedQueue {
public:
    explicit BoundedQueue(std::size_t capacity) : capacity_(capacity) {
        (void)capacity_;
    }

    // Block until the value can be accepted without violating capacity.
    void push(T value) {
        // TODO: implement
        (void)value;
    }

    // Block until the oldest value is available, then return it.
    T pop() {
        // TODO: implement
        return T{};
    }

    // Return a synchronized observation of the current item count.
    std::size_t size() const {
        // TODO: implement
        return 0;
    }

private:
    std::size_t capacity_;
    mutable std::mutex mutex_;
    std::condition_variable not_full_;
    std::condition_variable not_empty_;
    std::deque<T> items_;
};

int main() {
    // The unsynchronized observation is intentionally not asserted.
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
