// DRILL BRIEF
// Concept: A base pointer can use one virtual contract for different concrete types.
// Scenario: Poll camera and lidar sensors without branching on their concrete types.
// Implement: Camera::name, Camera::rate_hz, Lidar::name, Lidar::rate_hz, and poll.
// Behavior: Produce camera and lidar readings in the input order.
// Example: camera and lidar produce their asserted names and rates. Edge: an empty sensor list returns no readings.
// Interview focus: Separate an abstract interface from its implementations.
// Tests: main checks both virtual results and their order through Sensor pointers.
// Run: PRACTICE=1 uv run pytest ramp_up/cpp/04_virtual_functions_and_vtables/lessons/01_interfaces_and_dynamic_dispatch -q
// Done when: The test passes and the program prints ALL TESTS PASSED.

#include <cassert>
#include <iostream>
#include <string>
#include <vector>

struct Sensor {
    virtual ~Sensor() = default;
    virtual std::string name() const = 0;
    virtual double rate_hz() const = 0;
};

class Camera final : public Sensor {
public:
    std::string name() const override {
        // TODO: identify this backend.
        return "";
    }
    double rate_hz() const override {
        // TODO: report the camera rate.
        return 0.0;
    }
};

class Lidar final : public Sensor {
public:
    std::string name() const override {
        // TODO: identify this backend.
        return "";
    }
    double rate_hz() const override {
        // TODO: report the lidar rate.
        return 0.0;
    }
};

std::vector<std::string> poll(const std::vector<const Sensor*>& sensors) {
    // TODO: call the common contract for every borrowed sensor.
    (void)sensors;
    return {};
}

int main() {
    Camera camera;
    Lidar lidar;
    const std::vector<const Sensor*> sensors{&camera, &lidar};
    assert((poll(sensors) ==
            std::vector<std::string>{"camera=30.000000", "lidar=10.000000"}));
    std::cout << "ALL TESTS PASSED\n";
}
