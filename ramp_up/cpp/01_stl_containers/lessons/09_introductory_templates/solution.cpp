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
    if (samples.empty()) {
        return nullptr;
    }
    return &samples.back();
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
