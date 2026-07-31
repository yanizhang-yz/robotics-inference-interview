#include <algorithm>
#include <cassert>
#include <iostream>

double clamp_joint_command(double command, double lower, double upper) {
    assert(lower <= upper);
    return std::clamp(command, lower, upper);
}

int main() {
    assert(clamp_joint_command(0.4, -1.5, 1.5) == 0.4);
    assert(clamp_joint_command(2.0, -1.5, 1.5) == 1.5);
    assert(clamp_joint_command(-2.0, -1.5, 1.5) == -1.5);
    std::cout << "ALL TESTS PASSED\n";
}
