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

// Test-only observability: this friend probe reads wait registration while holding
// the queue mutex. It is not part of ClosableQueue's public teaching interface.
template <typename T>
struct ClosableQueueWaitProbe {
    static std::size_t blocked_consumers(ClosableQueue<T>& queue) {
        std::lock_guard<std::mutex> lock(queue.mutex_);
        return queue.waiting_consumers_;
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
            ++waiting_consumers_;
            not_empty_.wait(lock, [this] { return closed_ || !items_.empty(); });
            --waiting_consumers_;
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
    // Test-only count of consumers currently registered in the blocking wait.
    std::size_t waiting_consumers_ = 0;
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
        std::vector<std::thread> consumers;
        for (int i = 0; i < 3; ++i) {
            consumers.emplace_back([&] {
                if (!queue.pop().has_value()) {
                    stopped.fetch_add(1);
                }
            });
        }

        std::size_t blocked_consumers = 0;
        for (int poll = 0; poll < 100000 && blocked_consumers < 3; ++poll) {
            blocked_consumers =
                ClosableQueueWaitProbe<int>::blocked_consumers(queue);
            std::this_thread::yield();
        }
        assert(blocked_consumers == 3);
        queue.close();

        for (std::thread& consumer : consumers) {
            consumer.join();
        }
        assert(stopped.load() == 3);
    }
    std::cout << "ALL TESTS PASSED\n";
}
