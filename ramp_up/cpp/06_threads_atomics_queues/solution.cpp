// 06_threads_atomics_queues — reference solution.
//
// Compile & run standalone (note the extra -pthread vs earlier lessons):
//   clang++ -std=c++17 -Wall -pthread -o /tmp/threads solution.cpp && /tmp/threads
// Prints ALL TESTS PASSED when every assert holds.

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
// threads each ++ 100000 times. Correct answer: 200000. Actual answer: wrong and
// DIFFERENT almost every run (one run gave 143412) because ++ is three steps
// (read, add, write back) and the threads overwrite each other's writes.
// main() prints this value but never asserts on it: a data race is undefined
// behavior — there is no result you may rely on, not even a reliably wrong one.
// JAVA: the same bug: a plain int field, two threads running count++.
// C++:  same shape — the point of the demo is to SEE the lost updates.
int racy_increment_demo() {
    int counter = 0;  // shared by both threads, protected by nothing
    auto work = [&counter] {
        for (int i = 0; i < 100000; ++i) {
            ++counter;  // read-modify-write: three steps a thread can be interrupted between
        }
    };
    std::thread t1(work);
    std::thread t2(work);
    t1.join();
    t2.join();
    return counter;
}

// safe_count_mutex(4, 25000) -> exactly 100000, every run.
// JAVA: synchronized (lock) { count++; }
// C++:  std::lock_guard locks the mutex in its constructor and unlocks in its
//       destructor (RAII, lesson 02) — the guard's closing brace is the end of
//       the synchronized block, and the unlock runs even if the body throws.
int safe_count_mutex(int threads, int iters) {
    int counter = 0;
    std::mutex m;
    std::vector<std::thread> pool;  // threads are move-only, like unique_ptr; a vector of them is fine
    for (int t = 0; t < threads; ++t) {
        pool.emplace_back([&counter, &m, iters] {
            for (int i = 0; i < iters; ++i) {
                std::lock_guard<std::mutex> lock(m);  // critical section starts here
                ++counter;
            }                                         // ...and ends here: lock released
        });
    }
    for (auto& th : pool) {
        th.join();  // join EVERY thread — a joinable thread's destructor calls std::terminate
    }
    return counter;
}

// safe_count_atomic(4, 25000) -> exactly 100000, every run — no mutex involved.
// JAVA: AtomicInteger count; count.incrementAndGet();
// C++:  ++ on std::atomic<int> compiles to one indivisible hardware instruction;
//       no thread can slip in between the read and the write.
int safe_count_atomic(int threads, int iters) {
    std::atomic<int> counter{0};
    std::vector<std::thread> pool;
    for (int t = 0; t < threads; ++t) {
        pool.emplace_back([&counter, iters] {
            for (int i = 0; i < iters; ++i) {
                ++counter;  // atomic read-modify-write, lock-free
            }
        });
    }
    for (auto& th : pool) {
        th.join();
    }
    return counter.load();  // .load() = AtomicInteger.get(): an explicit atomic read
}

// A bounded, blocking, thread-safe FIFO queue — Java's ArrayBlockingQueue<T>.
// push() blocks while the queue is full; pop() blocks while it is empty.
// This is the camera->inference handoff: the camera thread push()es frames, the
// inference thread pop()s them, and when inference falls behind, the full queue
// puts the camera thread to sleep (backpressure) instead of letting frames pile
// up in memory without bound.
// JAVA: ArrayBlockingQueue — put() blocks when full, take() blocks when empty.
template <typename T>
class BoundedQueue {
public:
    explicit BoundedQueue(std::size_t capacity) : capacity_(capacity) {}

    // Blocks while the queue is full. JAVA: ArrayBlockingQueue.put(value).
    void push(T value) {
        std::unique_lock<std::mutex> lock(mutex_);  // unique_lock, not lock_guard: wait() needs it
        not_full_.wait(lock, [this] { return items_.size() < capacity_; });
        items_.push_back(std::move(value));
        not_empty_.notify_one();  // if a pop() is sleeping, wake it: there's an item now
    }

    // Blocks while the queue is empty. JAVA: ArrayBlockingQueue.take().
    T pop() {
        std::unique_lock<std::mutex> lock(mutex_);
        not_empty_.wait(lock, [this] { return !items_.empty(); });
        T value = std::move(items_.front());
        items_.pop_front();
        not_full_.notify_one();  // if a push() is sleeping, wake it: there's room now
        return value;
    }

    std::size_t size() const {
        std::lock_guard<std::mutex> lock(mutex_);  // no wait() here, so lock_guard suffices
        return items_.size();
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
