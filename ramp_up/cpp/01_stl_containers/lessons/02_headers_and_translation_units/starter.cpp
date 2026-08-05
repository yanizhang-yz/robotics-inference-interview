// DRILL BRIEF
// Concept: Share a declaration across translation units and keep a definition in one source file.
// Scenario: Check a command against the shared joint safety limits.
// Implement: command_is_safe.
// Behavior: with limits {-1.5, 1.5}, command_is_safe(0.5, limits) is true and command_is_safe(2.0, limits) is false; clamp_to_limits(2.0, limits) == 1.5.
// Interview focus: Implement inclusive limit checking in command_is_safe.
// Tests: main asserts the safe, unsafe, and linked helper cases.
// Run: PRACTICE=1 uv run pytest ramp_up/cpp/01_stl_containers/lessons/02_headers_and_translation_units -q
// Done when: The test run passes and the program prints ALL TESTS PASSED.

#include "joint_limits.hpp"

#include <cassert>
#include <iostream>

bool command_is_safe(double command, const JointLimits& limits) {
    // Exercise: decide whether the command is inside the inclusive safe range.
    (void)command;
    (void)limits;
    return false;
}

int main() {
    const JointLimits limits{-1.5, 1.5};
    assert(command_is_safe(0.5, limits));
    assert(!command_is_safe(2.0, limits));
    assert(clamp_to_limits(2.0, limits) == 1.5);
    std::cout << "ALL TESTS PASSED\n";
}
