#include "joint_limits.hpp"

#include <cassert>
#include <iostream>

bool command_is_safe(double command, const JointLimits& limits) {
    return command >= limits.lower && command <= limits.upper;
}

int main() {
    const JointLimits limits{-1.5, 1.5};
    assert(command_is_safe(0.5, limits));
    assert(!command_is_safe(2.0, limits));
    assert(clamp_to_limits(2.0, limits) == 1.5);
    std::cout << "ALL TESTS PASSED\n";
}
