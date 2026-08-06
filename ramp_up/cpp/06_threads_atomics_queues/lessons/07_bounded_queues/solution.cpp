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

// Test-only synchronization. The producer reports wait registration from inside
// the full-queue protocol; its wrapper reports completion after push returns.
class ProducerTestHandshake {
public:
    enum class Outcome { wait_registered, producer_returned };

    void register_wait() {
        {
            std::lock_guard<std::mutex> lock(mutex_);
            wait_registered_ = true;
        }
        changed_.notify_one();
    }

    void producer_returned() {
        {
            std::lock_guard<std::mutex> lock(mutex_);
            producer_returned_ = true;
        }
        changed_.notify_one();
    }

    Outcome wait_for_registration_or_return() {
        std::unique_lock<std::mutex> lock(mutex_);
        changed_.wait(lock, [this] { return wait_registered_ || producer_returned_; });
        return wait_registered_ ? Outcome::wait_registered : Outcome::producer_returned;
    }

    bool producer_has_returned() {
        std::lock_guard<std::mutex> lock(mutex_);
        return producer_returned_;
    }

private:
    std::mutex mutex_;
    std::condition_variable changed_;
    bool wait_registered_ = false;
    bool producer_returned_ = false;
};

template <typename T>
struct BoundedQueueWaitProbe {
    static void attach(BoundedQueue<T>& queue, ProducerTestHandshake& handshake) {
        std::lock_guard<std::mutex> lock(queue.mutex_);
        queue.test_handshake_ = &handshake;
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
            if (test_handshake_ != nullptr) {
                test_handshake_->register_wait();
            }
            not_full_.wait(lock, [this] { return items_.size() < capacity_; });
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
    ProducerTestHandshake* test_handshake_ = nullptr;
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
        ProducerTestHandshake handshake;
        BoundedQueueWaitProbe<int>::attach(queue, handshake);
        std::thread producer([&] {
            queue.push(30);
            handshake.producer_returned();
        });

        assert(handshake.wait_for_registration_or_return() ==
               ProducerTestHandshake::Outcome::wait_registered);
        assert(!handshake.producer_has_returned());
        assert(queue.size() == 2);

        assert(queue.pop() == 10);
        producer.join();
        assert(handshake.producer_has_returned());

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
