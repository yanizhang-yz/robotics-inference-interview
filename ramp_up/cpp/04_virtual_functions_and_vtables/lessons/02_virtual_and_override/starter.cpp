// DRILL BRIEF
// Concept: virtual enables runtime dispatch; override verifies an exact signature match.
// Scenario: Repair camera and lidar implementations used through const Sensor references.
// Implement: All four concrete name and rate methods while preserving every override.
// Behavior: Base references dispatch to each concrete backend's values.
// Interview focus: Explain virtual and use override to catch signature mistakes.
// Tests: main calls both implementations only through const Sensor references.
// Run: PRACTICE=1 uv run pytest ramp_up/cpp/04_virtual_functions_and_vtables/lessons/02_virtual_and_override -q
// Done when: The test passes and the program prints ALL TESTS PASSED.

#include <cassert>
#include <iostream>
#include <string>

struct Sensor {
    virtual ~Sensor() = default;
    virtual std::string name() const = 0;
    virtual double rate_hz() const = 0;
};

class Camera final : public Sensor {
public:
    std::string name() const override {
        // TODO: return the camera identity.
        return "";
    }
    double rate_hz() const override {
        // TODO: return the camera rate.
        return 0.0;
    }
};

class Lidar final : public Sensor {
public:
    std::string name() const override {
        // TODO: return the lidar identity.
        return "";
    }
    double rate_hz() const override {
        // TODO: return the lidar rate.
        return 0.0;
    }
};

int main() {
    Camera camera;
    Lidar lidar;
    const Sensor& camera_sensor = camera;
    const Sensor& lidar_sensor = lidar;
    assert(camera_sensor.name() == "camera");
    assert(camera_sensor.rate_hz() == 30.0);
    assert(lidar_sensor.name() == "lidar");
    assert(lidar_sensor.rate_hz() == 10.0);
    std::cout << "ALL TESTS PASSED\n";
}
