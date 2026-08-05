// DRILL BRIEF
// Concept: Shutdown is a protected state transition in the queue protocol.
// Scenario: Inference consumers must drain frames and then stop without hanging.
// Implement: ClosableQueue<T>::push, pop, and close.
// Behavior: Reject pushes after close; pop waits for closed-or-not-empty, drains
// queued items before nullopt, and close wakes every consumer with notify_all.
// Interview focus: Include shutdown in the wait predicate and name every waiter
// that the state change must wake; this unbounded queue has no producer wait.
// Tests: Push rejection, drain order, terminal nullopt, and three consumer exits.
// Run: PRACTICE=1 uv run pytest ramp_up/cpp/06_threads_atomics_queues/lessons/08_clean_shutdown -q
// Done when: The binary prints ALL TESTS PASSED.

#include <atomic>
#include <cassert>
#include <chrono>
#include <condition_variable>
#include <deque>
#include <iostream>
#include <mutex>
#include <optional>
#include <thread>
#include <utility>
#include <vector>

template <typename T>
class ClosableQueue {
public:
    bool push(T value) {
        // TODO: reject closed_, otherwise enqueue and notify one consumer.
        (void)value;
        return false;  // Neutral on purpose: incomplete practice fails quickly.
    }

    std::optional<T> pop() {
        // TODO: wait for closed_ || !items_.empty(), drain before nullopt.
        return std::nullopt;  // Neutral on purpose: never wait while incomplete.
    }

    void close() {
        // TODO: set closed_ under mutex_, then notify_all on not_empty_.
    }

private:
    std::mutex mutex_;
    std::condition_variable not_empty_;
    std::deque<T> items_;
    bool closed_ = false;
};

int main() {
    {
        ClosableQueue<int> queue;
        assert(queue.push(10));
        assert(queue.push(20));
        queue.close();
        assert(!queue.push(30));

        assert(queue.pop() == std::optional<int>{10});
        assert(queue.pop() == std::optional<int>{20});
        assert(!queue.pop().has_value());
    }

    {
        ClosableQueue<int> queue;
        std::atomic<int> started{0};
        std::atomic<int> stopped{0};
        std::vector<std::thread> consumers;
        for (int i = 0; i < 3; ++i) {
            consumers.emplace_back([&] {
                started.fetch_add(1);
                if (!queue.pop().has_value()) {
                    stopped.fetch_add(1);
                }
            });
        }

        for (int poll = 0; poll < 1000 && started.load() < 3; ++poll) {
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }
        assert(started.load() == 3);
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
        queue.close();

        for (std::thread& consumer : consumers) {
            consumer.join();
        }
        assert(stopped.load() == 3);
    }
    std::cout << "ALL TESTS PASSED\n";
}
