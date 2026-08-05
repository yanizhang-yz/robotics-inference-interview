// DRILL BRIEF
// Concept: Observe vector growth through capacity changes and use reserve.
// Scenario: Build a predictable per-frame batch without mid-build growth.
// Implement: count_capacity_changes and the two reallocation wrappers.
// Behavior: Count zero changes for empty input, one with reserve, and many without.
// Interview focus: Connect growth, amortized complexity, and borrow invalidation.
// Tests: main checks zero, reserved, and unreserved capacity-change counts.
// Run: PRACTICE=1 uv run pytest ramp_up/cpp/05_memory_layout_and_cache/lessons/06_allocation_and_reserve -q
// Done when: The test passes and the program prints ALL TESTS PASSED.

#include <cassert>
#include <cstddef>
#include <iostream>
#include <vector>

std::size_t count_capacity_changes(std::size_t n, bool reserve_first) {
    // TODO: build n integers and count every capacity change from zero.
    (void)n;
    (void)reserve_first;
    return 0;
}

std::size_t reallocations_with_reserve(std::size_t n) {
    // TODO: delegate to the counter with one up-front reserve.
    (void)n;
    return 0;
}

std::size_t reallocations_without_reserve(std::size_t n) {
    // TODO: delegate to the counter without reserving.
    (void)n;
    return 0;
}

int main() {
    assert(reallocations_with_reserve(0) == 0);
    assert(reallocations_without_reserve(0) == 0);
    assert(reallocations_with_reserve(100) == 1);
    assert(reallocations_without_reserve(100) > 1);
    std::cout << "ALL TESTS PASSED\n";
}
