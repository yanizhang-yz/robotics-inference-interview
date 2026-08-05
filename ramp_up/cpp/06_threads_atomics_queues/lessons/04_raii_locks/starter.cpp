// DRILL BRIEF
// Concept: RAII releases a mutex on every scope exit.
// Scenario: A sensor read throws while a timed mutex protects shared state.
// Implement: lock_released_after_exception(std::timed_mutex&).
// Behavior: Acquire with lock_guard, throw, catch outside the guard's scope,
// prove try_lock succeeds afterward, and unlock the successful try_lock.
// Interview focus: Explain why manual lock/unlock leaks locks on exceptional or
// early-return paths and how the guard's lifetime defines the critical section.
// Tests: The same mutex can be acquired after the exception.
// Run: PRACTICE=1 uv run pytest ramp_up/cpp/06_threads_atomics_queues/lessons/04_raii_locks -q
// Done when: The binary prints ALL TESTS PASSED.

#include <cassert>
#include <iostream>
#include <mutex>
#include <stdexcept>

bool lock_released_after_exception(std::timed_mutex& mutex) {
    // TODO: use lock_guard inside try, catch outside its scope, then try_lock.
    (void)mutex;
    return false;
}

int main() {
    std::timed_mutex mutex;
    assert(lock_released_after_exception(mutex));
    std::cout << "ALL TESTS PASSED\n";
}
