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

// Test-only synchronization. Consumers report wait registration from inside the
// empty-queue protocol and report completion after pop returns.
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
        std::lock_guard<std::mutex> lock(mutex_);
        if (closed_) {
            return false;
        }
        items_.push_back(std::move(value));
        not_empty_.notify_one();
        return true;
    }

    std::optional<T> pop() {
        std::unique_lock<std::mutex> lock(mutex_);
        if (!closed_ && items_.empty()) {
            if (test_handshake_ != nullptr) {
                test_handshake_->register_wait();
            }
            not_empty_.wait(lock, [this] { return closed_ || !items_.empty(); });
        }
        if (items_.empty()) {
            return std::nullopt;
        }
        T value = std::move(items_.front());
        items_.pop_front();
        return value;
    }

    void close() {
        {
            std::lock_guard<std::mutex> lock(mutex_);
            closed_ = true;
        }
        not_empty_.notify_all();
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
