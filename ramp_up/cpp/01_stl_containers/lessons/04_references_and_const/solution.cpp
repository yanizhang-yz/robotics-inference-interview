#include <algorithm>
#include <cassert>
#include <cmath>
#include <iostream>
#include <vector>

struct JointState {
    std::vector<double> positions;
};

double max_abs_position(const JointState& state) {
    double maximum = 0.0;
    for (double position : state.positions) {
        maximum = std::max(maximum, std::abs(position));
    }
    return maximum;
}

void clamp_in_place(JointState& state, double limit) {
    for (double& position : state.positions) {
        position = std::clamp(position, -limit, limit);
    }
}

int main() {
    JointState state{{-2.0, 0.5, 3.0}};
    assert(max_abs_position(state) == 3.0);
    clamp_in_place(state, 1.0);
    assert((state.positions == std::vector<double>{-1.0, 0.5, 1.0}));
    std::cout << "ALL TESTS PASSED\n";
}
