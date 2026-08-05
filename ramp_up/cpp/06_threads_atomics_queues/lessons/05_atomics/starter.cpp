// DRILL BRIEF
// Concept: Compare-and-swap elects exactly one winner for one state transition.
// Scenario: Many inference workers race to publish one shared result.
// Implement: elect_publisher(threads) with atomic<bool>::compare_exchange_strong.
// Behavior: Each worker starts with expected=false; exactly one changes claimed
// to true and increments the atomic winner count; all workers are joined.
// Interview focus: Explain CAS expected-value updates and when a mutex is clearer
// for invariants spanning more than one field.
// Tests: One worker and eight workers both elect exactly one publisher.
// Run: PRACTICE=1 uv run pytest ramp_up/cpp/06_threads_atomics_queues/lessons/05_atomics -q
// Done when: The binary prints ALL TESTS PASSED.

#include <atomic>
#include <cassert>
#include <iostream>
#include <thread>
#include <vector>

int elect_publisher(int threads) {
    // TODO: race workers through compare_exchange_strong and count one winner.
    (void)threads;
    return 0;
}

int main() {
    assert(elect_publisher(1) == 1);
    assert(elect_publisher(8) == 1);
    std::cout << "ALL TESTS PASSED\n";
}
