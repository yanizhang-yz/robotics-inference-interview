// DRILL BRIEF
// Concept: A condition variable waits for a protected state predicate.
// Scenario: A camera producer delivers one sample to a sleeping consumer.
// Implement: SampleMailbox<T>::put, wait_and_take, and the locked empty query.
// Behavior: put changes optional state under the mutex then notifies; take waits
// with a has_value predicate, moves the value out, and resets the optional.
// Interview focus: Identify the mutex, predicate, state transition, notification,
// and why predicate waits tolerate spurious wakeups.
// Tests: A producer/consumer handoff delivers 42 and leaves the mailbox empty.
// Run: PRACTICE=1 uv run pytest ramp_up/cpp/06_threads_atomics_queues/lessons/06_condition_variables -q
// Done when: The binary prints ALL TESTS PASSED.

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
        // TODO: store under mutex_, then notify ready_.
        (void)value;
        (void)mutex_;
        (void)ready_;
        (void)value_;
    }

    T wait_and_take() {
        // TODO: wait with a value_.has_value() predicate, then move and reset.
        return T{};  // Neutral on purpose: incomplete practice fails, never hangs.
    }

    bool empty() const {
        // TODO: lock mutex_ and inspect value_.
        return true;
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
