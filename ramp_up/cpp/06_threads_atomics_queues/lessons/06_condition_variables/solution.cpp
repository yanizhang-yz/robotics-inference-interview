#include <cassert>
#include <condition_variable>
#include <iostream>
#include <mutex>
#include <optional>
#include <thread>
#include <utility>

// Test-only synchronization. It reports either real wait registration or an
// early worker return without relying on sleeps, yields, or elapsed time.
class MailboxTestHandshake {
public:
    enum class Outcome { wait_registered, consumer_returned };

    void register_wait() {
        {
            std::lock_guard<std::mutex> lock(mutex_);
            wait_registered_ = true;
        }
        changed_.notify_one();
    }

    void consumer_returned() {
        {
            std::lock_guard<std::mutex> lock(mutex_);
            consumer_returned_ = true;
        }
        changed_.notify_one();
    }

    Outcome wait_for_registration_or_return() {
        std::unique_lock<std::mutex> lock(mutex_);
        changed_.wait(lock, [this] { return wait_registered_ || consumer_returned_; });
        return wait_registered_ ? Outcome::wait_registered : Outcome::consumer_returned;
    }

private:
    std::mutex mutex_;
    std::condition_variable changed_;
    bool wait_registered_ = false;
    bool consumer_returned_ = false;
};

template <typename T>
struct SampleMailboxTestProbe;

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
        if (!value_.has_value() && test_handshake_ != nullptr) {
            test_handshake_->register_wait();
        }
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
    friend struct SampleMailboxTestProbe<T>;

    mutable std::mutex mutex_;
    std::condition_variable ready_;
    std::optional<T> value_;
    MailboxTestHandshake* test_handshake_ = nullptr;
};

template <typename T>
struct SampleMailboxTestProbe {
    static void attach(SampleMailbox<T>& mailbox, MailboxTestHandshake& handshake) {
        std::lock_guard<std::mutex> lock(mailbox.mutex_);
        mailbox.test_handshake_ = &handshake;
    }
};

int main() {
    SampleMailbox<int> mailbox;
    MailboxTestHandshake handshake;
    SampleMailboxTestProbe<int>::attach(mailbox, handshake);
    int received = 0;

    std::thread consumer([&] {
        received = mailbox.wait_and_take();
        handshake.consumer_returned();
    });
    assert(handshake.wait_for_registration_or_return() ==
           MailboxTestHandshake::Outcome::wait_registered);
    mailbox.put(42);
    consumer.join();

    assert(received == 42);
    assert(mailbox.empty());
    std::cout << "ALL TESTS PASSED\n";
}
