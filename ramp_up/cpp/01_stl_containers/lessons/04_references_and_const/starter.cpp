// DRILL BRIEF
// Concept: Read caller-owned data through const references and mutate it through references.
// Scenario: Inspect a joint state, then clamp its positions in place.
// Implement: max_abs_position and clamp_in_place.
// Behavior: max_abs_position({-2.0, 0.5, 3.0}) == 3.0; after clamp_in_place(state, 1.0), positions are {-1.0, 0.5, 1.0}.
// Interview focus: Read by const& in max_abs_position and mutate by & in clamp_in_place.
// Tests: main checks the maximum magnitude and the caller-visible clamped vector.
// Run: PRACTICE=1 uv run pytest ramp_up/cpp/01_stl_containers/lessons/04_references_and_const -q
// Done when: The test run passes and the program prints ALL TESTS PASSED.

#include <algorithm>
#include <cassert>
#include <cmath>
#include <iostream>
#include <vector>

struct JointState {
    std::vector<double> positions;
};

double max_abs_position(const JointState& state) {
    // Exercise: inspect the state without changing the caller's positions.
    (void)state;
    return 0.0;
}

void clamp_in_place(JointState& state, double limit) {
    // Exercise: clamp each caller-owned position to [-limit, limit].
    (void)state;
    (void)limit;
}

int main() {
    JointState state{{-2.0, 0.5, 3.0}};
    assert(max_abs_position(state) == 3.0);
    clamp_in_place(state, 1.0);
    assert((state.positions == std::vector<double>{-1.0, 0.5, 1.0}));
    std::cout << "ALL TESTS PASSED\n";
}
