// DRILL BRIEF
// Concept: A mutex protects a shared invariant from data races.
// Scenario: Several inference workers update one completed-work counter.
// Implement: safe_count_mutex(threads, iterations) with a plain int and mutex.
// Behavior: Lock around every increment, join every worker, return the exact sum.
// Interview focus: Define a data race and name the invariant and lock boundary.
// Tests: Four workers times 25000 and one worker times 1000 are exact.
// Run: PRACTICE=1 uv run pytest ramp_up/cpp/06_threads_atomics_queues/lessons/03_data_races_and_mutexes -q
// Done when: The binary prints ALL TESTS PASSED.

#include <cassert>
#include <iostream>
#include <mutex>
#include <thread>
#include <vector>

int safe_count_mutex(int threads, int iterations) {
    // TODO: protect a plain counter with one mutex and join every worker.
    (void)threads;
    (void)iterations;
    return 0;
}

int main() {
    assert(safe_count_mutex(4, 25000) == 100000);
    assert(safe_count_mutex(1, 1000) == 1000);
    std::cout << "ALL TESTS PASSED\n";
}
