#include <cassert>
#include <cstdint>
#include <iostream>
#include <optional>
#include <vector>

struct JointSample {
    std::int64_t timestamp_ns;
    double position;
};

std::optional<JointSample> latest_at_or_before(
    const std::vector<JointSample>& samples,
    std::int64_t timestamp_ns
) {
    // Exercise: return the latest sample at or before timestamp_ns.
    (void)samples;
    (void)timestamp_ns;
    return std::nullopt;
}

int main() {
    const std::vector<JointSample> samples{{10, 0.1}, {20, 0.2}, {30, 0.3}};
    const auto found = latest_at_or_before(samples, 25);
    assert(found.has_value());
    assert(found->timestamp_ns == 20);
    assert(!latest_at_or_before(samples, 5).has_value());

    const std::vector<JointSample> out_of_order_samples{{20, 0.2}, {10, 0.1}};
    const auto latest = latest_at_or_before(out_of_order_samples, 25);
    assert(latest.has_value());
    assert(latest->timestamp_ns == 20);
    std::cout << "ALL TESTS PASSED\n";
}
