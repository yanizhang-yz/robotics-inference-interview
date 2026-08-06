// DRILL BRIEF
// Concept: Shutdown is a protected state transition in the queue protocol.
// Scenario: Inference consumers must drain frames and then stop without hanging.
// Implement: ClosableQueue<T>::push, pop, and close.
// Behavior: Reject pushes after close, drain accepted items before nullopt, and
// Example: after close, queued 10 and 20 drain before nullopt. Edge: later pushes fail and every empty waiter exits.
// release every consumer blocked when shutdown begins.
// Interview focus: Include shutdown in the wait predicate and name every waiter
// that the state change must wake; this unbounded queue has no producer wait.
// Tests: Push rejection, drain order, terminal nullopt, and a test-only handshake
// that distinguishes three wait registrations from any early consumer return.
// Run: PRACTICE=1 uv run pytest ramp_up/cpp/06_threads_atomics_queues/lessons/08_clean_shutdown -q
// Done when: The binary prints ALL TESTS PASSED.

#include <atomic>
#include <cassert>
#include <condition_variable>
#include <cstddef>
#include <deque>
#include <iostream>
#include <mutex>
#include <optional>
#include <thread>
#include <utility>
#include <vector>

template <typename T>
class ClosableQueue;

// Test-only synchronization. It is separate from the queue's public interface and
// protects its own counters with a mutex and condition variable.
class ConsumerTestHandshake {
public:
    enum class Outcome { all_waits_registered, consumer_returned };

    void register_wait() {
        {
            std::lock_guard<std::mutex> lock(mutex_);
            ++registered_;
        }
        changed_.notify_one();
    }

    void consumer_returned() {
        {
            std::lock_guard<std::mutex> lock(mutex_);
            ++returned_;
        }
        changed_.notify_one();
    }

    Outcome wait_for_registrations_or_return(std::size_t expected) {
        std::unique_lock<std::mutex> lock(mutex_);
        changed_.wait(lock, [this, expected] {
            return registered_ == expected || returned_ != 0;
        });
        return registered_ == expected ? Outcome::all_waits_registered
                                       : Outcome::consumer_returned;
    }

    std::size_t returned_count() {
        std::lock_guard<std::mutex> lock(mutex_);
        return returned_;
    }

private:
    std::mutex mutex_;
    std::condition_variable changed_;
    std::size_t registered_ = 0;
    std::size_t returned_ = 0;
};

template <typename T>
struct ClosableQueueWaitProbe {
    static void attach(ClosableQueue<T>& queue, ConsumerTestHandshake& handshake) {
        std::lock_guard<std::mutex> lock(queue.mutex_);
        queue.test_handshake_ = &handshake;
    }
};

template <typename T>
class ClosableQueue {
public:
    bool push(T value) {
        // TODO: implement push.
        (void)value;
        return false;  // Neutral on purpose: incomplete practice fails quickly.
    }

    std::optional<T> pop() {
        // TODO: implement pop.
        return std::nullopt;  // Neutral on purpose: never wait while incomplete.
    }

    void close() {
        // TODO: implement close.
    }

private:
    friend struct ClosableQueueWaitProbe<T>;

    std::mutex mutex_;
    std::condition_variable not_empty_;
    std::deque<T> items_;
    bool closed_ = false;
    ConsumerTestHandshake* test_handshake_ = nullptr;
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
        std::atomic<int> stopped{0};
        ConsumerTestHandshake handshake;
        ClosableQueueWaitProbe<int>::attach(queue, handshake);
        std::vector<std::thread> consumers;
        for (int i = 0; i < 3; ++i) {
            consumers.emplace_back([&] {
                if (!queue.pop().has_value()) {
                    stopped.fetch_add(1);
                }
                handshake.consumer_returned();
            });
        }

        assert(handshake.wait_for_registrations_or_return(3) ==
               ConsumerTestHandshake::Outcome::all_waits_registered);
        queue.close();

        for (std::thread& consumer : consumers) {
            consumer.join();
        }
        assert(handshake.returned_count() == 3);
        assert(stopped.load() == 3);
    }
    std::cout << "ALL TESTS PASSED\n";
}
