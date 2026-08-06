// DRILL BRIEF
// Concept: std::thread lifetime and explicit ownership of the join boundary.
// Scenario: Sum two disjoint halves of an inference batch concurrently.
// Implement: parallel_sum(std::span<const int>) with two worker threads.
// Behavior: Two workers process disjoint portions and return the exact total.
// Example: the asserted positive and mixed-sign ranges sum exactly. Edge: empty input returns 0 without an invalid join.
// Interview focus: Explain when construction starts work and why every joinable
// thread must be joined or have its ownership transferred before destruction.
// Tests: Empty input is 0; positive and mixed-sign inputs have exact sums.
// Run: PRACTICE=1 uv run pytest ramp_up/cpp/06_threads_atomics_queues/lessons/01_thread_lifetime -q
// Done when: The binary prints ALL TESTS PASSED.

#include <cassert>
#include <cstddef>
#include <iostream>
#include <span>
#include <thread>
#include <vector>

long long parallel_sum(std::span<const int> values) {
    // TODO: implement parallel_sum.
    (void)values;
    return 0;
}

int main() {
    const std::vector<int> empty;
    const std::vector<int> positive{1, 2, 3, 4};
    const std::vector<int> mixed{-8, 3, -2, 10};

    assert(parallel_sum(empty) == 0);
    assert(parallel_sum(positive) == 10);
    assert(parallel_sum(mixed) == 3);
    std::cout << "ALL TESTS PASSED\n";
}
