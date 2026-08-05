// DRILL BRIEF
// Concept: Compile and link one self-testing C++ program.
// Scenario: Clamp a robot joint command before it reaches hardware.
// Implement: clamp_joint_command.
// Behavior: clamp_joint_command(0.4, -1.5, 1.5) == 0.4; (2.0, -1.5, 1.5) == 1.5; (-2.0, -1.5, 1.5) == -1.5.
// Interview focus: Repair clamp_joint_command so it constrains a requested joint command to its inclusive limits.
// Tests: The assertions in main cover an in-range command and both inclusive bounds.
// Run: PRACTICE=1 uv run pytest ramp_up/cpp/01_stl_containers/lessons/01_build_compile_and_link -q
// Done when: The test run passes and the program prints ALL TESTS PASSED.

#include <algorithm>
#include <cassert>
#include <iostream>

double clamp_joint_command(double command, double lower, double upper) {
    // Exercise: return the command clamped to the inclusive [lower, upper] range.
    (void)lower;
    (void)upper;
    return command;
}

int main() {
    assert(clamp_joint_command(0.4, -1.5, 1.5) == 0.4);
    assert(clamp_joint_command(2.0, -1.5, 1.5) == 1.5);
    assert(clamp_joint_command(-2.0, -1.5, 1.5) == -1.5);
    std::cout << "ALL TESTS PASSED\n";
}
