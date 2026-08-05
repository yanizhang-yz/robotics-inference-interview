// DRILL BRIEF
// Concept: Represent an eligible sample that may be absent with std::optional.
// Scenario: Find the newest joint sample available at a requested time.
// Implement: latest_at_or_before.
// Behavior: samples at 10, 20, and 30 return timestamp 20 for cutoff 25 and no value for cutoff 5; out-of-order {20, 10} still returns 20 for cutoff 25.
// Interview focus: Return the latest eligible sample from latest_at_or_before.
// Tests: main covers a match, absence, and an out-of-order input sequence.
// Run: PRACTICE=1 uv run pytest ramp_up/cpp/01_stl_containers/lessons/06_optional_results -q
// Done when: The test run passes and the program prints ALL TESTS PASSED.

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
