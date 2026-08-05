// DRILL BRIEF
// Concept: Reuse one template to select the newest sample of several value types.
// Scenario: Read the latest joint measurement or operating mode without copying either sample.
// Implement: latest_sample.
// Behavior: for std::vector<TimedSample<double>> joints{{10, 0.1}, {20, 0.2}}, latest_sample(joints)->value == 0.2; for std::vector<TimedSample<std::string>> modes{{15, "idle"}}, latest_sample(modes)->value == "idle"; latest_sample(std::vector<TimedSample<int>>{}) == nullptr.
// Interview focus: Find the latest typed sample.
// Tests: main instantiates the template for double, std::string, and int data.
// Run: PRACTICE=1 uv run pytest ramp_up/cpp/01_stl_containers/lessons/09_introductory_templates -q
// Done when: The test run passes and the program prints ALL TESTS PASSED.

#include <cassert>
#include <cstdint>
#include <iostream>
#include <string>
#include <vector>

template <typename T>
struct TimedSample {
    std::int64_t timestamp_ns;
    T value;
};

template <typename T>
const TimedSample<T>* latest_sample(
    const std::vector<TimedSample<T>>& samples
) {
    // Exercise: return a pointer to the newest sample, or nullptr when empty.
    (void)samples;
    return nullptr;
}

int main() {
    const std::vector<TimedSample<double>> joints{{10, 0.1}, {20, 0.2}};
    const std::vector<TimedSample<std::string>> modes{{15, "idle"}};
    const auto* latest_joint = latest_sample(joints);
    const auto* latest_mode = latest_sample(modes);
    assert(latest_joint != nullptr);
    assert(latest_mode != nullptr);
    assert(latest_joint->value == 0.2);
    assert(latest_mode->value == "idle");
    assert(latest_sample(std::vector<TimedSample<int>>{}) == nullptr);
    std::cout << "ALL TESTS PASSED\n";
}
