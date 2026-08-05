#include <cassert>
#include <condition_variable>
#include <iostream>
#include <mutex>
#include <optional>
#include <thread>
#include <utility>

template <typename T>
class SampleMailbox {
public:
    void put(T value) {
        {
            std::lock_guard<std::mutex> lock(mutex_);
            value_ = std::move(value);
        }
        ready_.notify_one();
    }

    T wait_and_take() {
        std::unique_lock<std::mutex> lock(mutex_);
        ready_.wait(lock, [this] { return value_.has_value(); });
        T value = std::move(*value_);
        value_.reset();
        return value;
    }

    bool empty() const {
        std::lock_guard<std::mutex> lock(mutex_);
        return !value_.has_value();
    }

private:
    mutable std::mutex mutex_;
    std::condition_variable ready_;
    std::optional<T> value_;
};

int main() {
    SampleMailbox<int> mailbox;
    int received = 0;

    std::thread consumer([&] { received = mailbox.wait_and_take(); });
    mailbox.put(42);
    consumer.join();

    assert(received == 42);
    assert(mailbox.empty());
    std::cout << "ALL TESTS PASSED\n";
}
