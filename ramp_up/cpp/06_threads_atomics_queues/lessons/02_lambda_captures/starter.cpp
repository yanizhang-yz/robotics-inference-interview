// DRILL BRIEF
// Concept: Lambda capture modes determine sharing and lifetime.
// Scenario: Offset independent sensor samples in parallel without reordering.
// Implement: offset_samples(samples, offset), one worker per sample.
// Behavior: Capture index and offset by value, output by reference, write one
// disjoint output slot per worker, and join every worker before returning.
// Interview focus: Diagnose loop-index reference capture and dangling captures.
// Tests: The result preserves input order and handles an empty input.
// Run: PRACTICE=1 uv run pytest ramp_up/cpp/06_threads_atomics_queues/lessons/02_lambda_captures -q
// Done when: The binary prints ALL TESTS PASSED.

#include <cassert>
#include <cstddef>
#include <iostream>
#include <thread>
#include <vector>

std::vector<int> offset_samples(const std::vector<int>& samples, int offset) {
    // TODO: launch one worker per element with [&, i, offset], then join all.
    (void)samples;
    (void)offset;
    return {};
}

int main() {
    const std::vector<int> samples{4, -1, 9, 0};
    const std::vector<int> expected{10, 5, 15, 6};

    assert(offset_samples(samples, 6) == expected);
    assert(offset_samples({}, 3).empty());
    std::cout << "ALL TESTS PASSED\n";
}
