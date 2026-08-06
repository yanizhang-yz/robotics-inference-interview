// DRILL BRIEF
// Concept: std::span is a length-carrying, non-owning view of contiguous storage.
// Scenario: Scale a tensor slice and compute a read-only mean.
// Implement: scale_in_place and mean, including the empty-view case.
// Behavior: Mutate only a middle subspan; whole-range mean is 19.2; empty mean is 0.
// Example: only the asserted middle tensor values scale. Edge: an empty const span has mean 0.0.
// Interview focus: Choose span<T> vs span<const T> and state the lifetime requirement.
// Tests: main checks slice mutation, untouched edges, whole mean, and empty mean.
// Run: PRACTICE=1 uv run pytest ramp_up/cpp/02_ownership_and_raii/lessons/06_span_views -q
// Done when: The test run passes and the program prints ALL TESTS PASSED.

#include <cassert>
#include <cmath>
#include <iostream>
#include <span>
#include <vector>

void scale_in_place(std::span<float> values, float gain) {
    // Exercise: mutate every element visible through this view.
    (void)values;
    (void)gain;
}

double mean(std::span<const float> values) {
    // Exercise: implement the asserted mean contract.
    (void)values;
    return 0.0;
}

int main() {
    std::vector<float> tensor{1.0F, 2.0F, 3.0F, 4.0F, 5.0F};
    scale_in_place(std::span<float>(tensor).subspan(1, 3), 10.0F);
    assert((tensor == std::vector<float>{1.0F, 20.0F, 30.0F, 40.0F, 5.0F}));
    assert(std::abs(mean(tensor) - 19.2) < 1e-9);
    assert(mean(std::span<const float>{}) == 0.0);
    std::cout << "ALL TESTS PASSED\n";
}
