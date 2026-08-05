// DRILL BRIEF
// Concept: Return non-owning pointer access when a matching element exists.
// Scenario: Look up a named sensor in caller-owned telemetry configuration.
// Implement: find_sensor.
// Behavior: find_sensor(sensors, "wrist-camera") is non-null with rate_hz 30.0; find_sensor(sensors, "missing") == nullptr.
// Interview focus: Return the matching sensor from find_sensor, or nullptr when none matches.
// Tests: main asserts both the found camera data and the missing-sensor result.
// Run: PRACTICE=1 uv run pytest ramp_up/cpp/01_stl_containers/lessons/05_pointer_borrows -q
// Done when: The test run passes and the program prints ALL TESTS PASSED.

#include <cassert>
#include <iostream>
#include <string>
#include <vector>

struct Sensor {
    std::string name;
    double rate_hz;
};

const Sensor* find_sensor(
    const std::vector<Sensor>& sensors,
    const std::string& name
) {
    // Exercise: return non-owning access to the matching sensor, or nullptr.
    (void)sensors;
    (void)name;
    return nullptr;
}

int main() {
    const std::vector<Sensor> sensors{
        {"wrist-camera", 30.0},
        {"joint-encoder", 100.0},
    };
    const Sensor* camera = find_sensor(sensors, "wrist-camera");
    assert(camera != nullptr);
    assert(camera->rate_hz == 30.0);
    assert(find_sensor(sensors, "missing") == nullptr);
    std::cout << "ALL TESTS PASSED\n";
}
