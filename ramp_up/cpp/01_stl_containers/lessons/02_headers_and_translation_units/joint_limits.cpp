#include "joint_limits.hpp"

#include <algorithm>
#include <cassert>

double clamp_to_limits(double command, const JointLimits& limits) {
    assert(limits.lower <= limits.upper);
    return std::clamp(command, limits.lower, limits.upper);
}
