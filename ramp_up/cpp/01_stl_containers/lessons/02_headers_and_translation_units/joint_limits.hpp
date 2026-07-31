#pragma once

struct JointLimits {
    double lower;
    double upper;
};

double clamp_to_limits(double command, const JointLimits& limits);
