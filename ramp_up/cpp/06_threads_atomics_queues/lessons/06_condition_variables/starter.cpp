// DRILL BRIEF
// Concept: A condition variable waits for a protected state predicate.
// Scenario: A camera producer delivers one sample to a sleeping consumer.
// Implement: SampleMailbox<T>::put, wait_and_take, and the locked empty query.
// Behavior: Deliver one value without busy-waiting and leave the mailbox empty.
// Example: a consumer registered on an empty mailbox receives 42. Edge: an immediate non-waiting return fails before put.
// Interview focus: Identify the mutex, predicate, state transition, notification,
// and why predicate waits tolerate spurious wakeups.
// Tests: A test-only handshake proves the consumer reaches the empty wait before
// put; the handoff then delivers 42 and leaves the mailbox empty.
// Run: PRACTICE=1 uv run pytest ramp_up/cpp/06_threads_atomics_queues/lessons/06_condition_variables -q
// Done when: The binary prints ALL TESTS PASSED.

#include <cassert>
#include <condition_variable>
#include <iostream>
#include <mutex>
#include <optional>
#include <thread>
#include <utility>

// Test-only synchronization. It distinguishes wait registration from an
// incomplete method returning immediately; it is not part of the mailbox API.
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
        // TODO: implement put.
        (void)value;
        (void)mutex_;
        (void)ready_;
        (void)value_;
    }

    T wait_and_take() {
        // TODO: implement wait_and_take.
        return T{};  // Neutral on purpose: incomplete practice fails, never hangs.
    }

    bool empty() const {
        // TODO: implement empty.
        return true;
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
