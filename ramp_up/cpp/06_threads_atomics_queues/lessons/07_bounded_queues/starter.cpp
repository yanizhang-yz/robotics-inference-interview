// DRILL BRIEF
// Concept: A bounded blocking FIFO applies backpressure with two predicates.
// Scenario: A camera must stop producing while inference has filled the queue.
// Implement: BoundedQueue<T> constructor, push, pop, and locked size.
// Behavior: Reject zero capacity; push waits for space and notifies not_empty;
// pop waits for data and notifies not_full; both preserve FIFO and capacity.
// Interview focus: Derive the not-full/not-empty predicates and notifications,
// and explain why bounded capacity prevents unbounded latency and memory growth.
// Tests: Zero capacity, FIFO, capacity, producer blocking, and exact delivery.
// Run: PRACTICE=1 uv run pytest ramp_up/cpp/06_threads_atomics_queues/lessons/07_bounded_queues -q
// Done when: The binary prints ALL TESTS PASSED.

#include <atomic>
#include <cassert>
#include <chrono>
#include <condition_variable>
#include <cstddef>
#include <deque>
#include <initializer_list>
#include <iostream>
#include <mutex>
#include <stdexcept>
#include <thread>
#include <utility>

template <typename T>
class BoundedQueue {
public:
    explicit BoundedQueue(std::size_t capacity) : capacity_(capacity) {
        // TODO: reject zero with invalid_argument("capacity must be positive").
        (void)capacity_;
    }

    void push(T value) {
        // TODO: wait for items_.size() < capacity_, enqueue, notify not_empty_.
        (void)value;  // Neutral on purpose: incomplete practice never blocks.
    }

    T pop() {
        // TODO: wait for !items_.empty(), dequeue FIFO, notify not_full_.
        return T{};  // Neutral on purpose: incomplete practice never blocks.
    }

    std::size_t size() const {
        // TODO: lock mutex_ and return items_.size().
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
    bool rejected_zero = false;
    try {
        BoundedQueue<int> invalid(0);
    } catch (const std::invalid_argument&) {
        rejected_zero = true;
    }
    assert(rejected_zero);

    {
        BoundedQueue<int> queue(2);
        queue.push(10);
        queue.push(20);
        assert(queue.size() == 2);
        assert(queue.pop() == 10);
        assert(queue.pop() == 20);
        assert(queue.size() == 0);
    }

    {
        BoundedQueue<int> queue(2);
        std::atomic<int> progress{0};
        std::thread producer([&] {
            queue.push(10);
            progress.store(1);
            queue.push(20);
            progress.store(2);
            progress.store(3);  // The third push is about to be attempted.
            queue.push(30);
            progress.store(4);  // The third push completed.
        });

        for (int poll = 0; poll < 1000 && progress.load() < 3; ++poll) {
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }
        assert(progress.load() == 3);
        for (int poll = 0; poll < 20 && progress.load() == 3; ++poll) {
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }
        assert(progress.load() == 3);
        assert(queue.size() == 2);

        assert(queue.pop() == 10);
        for (int poll = 0; poll < 1000 && progress.load() < 4; ++poll) {
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }
        assert(progress.load() == 4);
        producer.join();

        assert(queue.size() == 2);
        assert(queue.pop() == 20);
        assert(queue.pop() == 30);
        assert(queue.size() == 0);
    }

    {
        BoundedQueue<int> queue(4);
        std::thread producer([&] {
            for (int value = 0; value < 100; ++value) {
                queue.push(value);
            }
        });
        for (int expected = 0; expected < 100; ++expected) {
            assert(queue.pop() == expected);
        }
        producer.join();
        assert(queue.size() == 0);
    }
    std::cout << "ALL TESTS PASSED\n";
}
