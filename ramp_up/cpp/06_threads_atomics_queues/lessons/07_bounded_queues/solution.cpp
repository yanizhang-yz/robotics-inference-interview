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
        if (capacity == 0) {
            throw std::invalid_argument("capacity must be positive");
        }
    }

    void push(T value) {
        std::unique_lock<std::mutex> lock(mutex_);
        not_full_.wait(lock, [this] { return items_.size() < capacity_; });
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
