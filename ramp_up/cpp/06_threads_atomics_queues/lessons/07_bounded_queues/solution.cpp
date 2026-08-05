#include <atomic>
#include <cassert>
#include <condition_variable>
#include <cstddef>
#include <deque>
#include <iostream>
#include <mutex>
#include <stdexcept>
#include <thread>
#include <utility>

template <typename T>
class BoundedQueue;

// Test-only observability: this friend probe reads wait registration while holding
// the queue mutex. It is not part of BoundedQueue's public teaching interface.
template <typename T>
struct BoundedQueueWaitProbe {
    static std::size_t blocked_producers(const BoundedQueue<T>& queue) {
        std::lock_guard<std::mutex> lock(queue.mutex_);
        return queue.waiting_producers_;
    }
};

template <typename T>
class BoundedQueue {
public:
    explicit BoundedQueue(std::size_t capacity) : capacity_(capacity) {
        if (capacity == 0) {
            throw std::invalid_argument("capacity must be positive");
        }
    }

    void push(T value) {
        std::unique_lock<std::mutex> lock(mutex_);
        if (items_.size() >= capacity_) {
            ++waiting_producers_;
            not_full_.wait(lock, [this] { return items_.size() < capacity_; });
            --waiting_producers_;
        }
        items_.push_back(std::move(value));
        not_empty_.notify_one();
    }

    T pop() {
        std::unique_lock<std::mutex> lock(mutex_);
        not_empty_.wait(lock, [this] { return !items_.empty(); });
        T value = std::move(items_.front());
        items_.pop_front();
        not_full_.notify_one();
        return value;
    }

    std::size_t size() const {
        std::lock_guard<std::mutex> lock(mutex_);
        return items_.size();
    }

private:
    friend struct BoundedQueueWaitProbe<T>;

    std::size_t capacity_;
    mutable std::mutex mutex_;
    std::condition_variable not_full_;
    std::condition_variable not_empty_;
    std::deque<T> items_;
    // Test-only count of producers currently registered in the blocking wait.
    std::size_t waiting_producers_ = 0;
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
        queue.push(10);
        queue.push(20);
        std::atomic<bool> third_push_completed{false};
        std::thread producer([&] {
            queue.push(30);
            third_push_completed.store(true);
        });

        bool producer_is_blocked = false;
        for (int poll = 0; poll < 100000 && !producer_is_blocked; ++poll) {
            producer_is_blocked =
                BoundedQueueWaitProbe<int>::blocked_producers(queue) == 1;
            std::this_thread::yield();
        }
        assert(producer_is_blocked);
        assert(!third_push_completed.load());
        assert(queue.size() == 2);

        assert(queue.pop() == 10);
        for (int poll = 0; poll < 100000 && !third_push_completed.load(); ++poll) {
            std::this_thread::yield();
        }
        assert(third_push_completed.load());
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
