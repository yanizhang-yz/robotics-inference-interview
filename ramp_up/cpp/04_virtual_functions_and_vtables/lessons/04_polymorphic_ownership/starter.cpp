// DRILL BRIEF
// Concept: unique_ptr<Sensor> owns one concrete backend behind a common interface.
// Scenario: Build and aggregate a mixed collection of camera and lidar sensors.
// Implement: make_sensors and total_rate using unique_ptr and virtual calls.
// Behavior: Return both dynamic types, total 40 Hz, and clean everything at scope exit.
// Interview focus: Explain how a homogeneous vector can own heterogeneous pointees.
// Tests: main checks type behavior, total rate, live count, and automatic cleanup.
// Run: PRACTICE=1 uv run pytest ramp_up/cpp/04_virtual_functions_and_vtables/lessons/04_polymorphic_ownership -q
// Done when: The test passes and the program prints ALL TESTS PASSED.

#include <cassert>
#include <iostream>
#include <memory>
#include <string>
#include <vector>

struct Sensor {
    inline static int live_objects = 0;

    Sensor() { ++live_objects; }
    virtual ~Sensor() { --live_objects; }
    virtual std::string name() const = 0;
    virtual double rate_hz() const = 0;
};

class Camera final : public Sensor {
public:
    std::string name() const override { return "camera"; }
    double rate_hz() const override { return 30.0; }
};

class Lidar final : public Sensor {
public:
    std::string name() const override { return "lidar"; }
    double rate_hz() const override { return 10.0; }
};

std::vector<std::unique_ptr<Sensor>> make_sensors() {
    // TODO: transfer two concrete owners into one homogeneous vector.
    return {};
}

double total_rate(const std::vector<std::unique_ptr<Sensor>>& sensors) {
    // TODO: borrow each owner and dispatch through Sensor.
    (void)sensors;
    return 0.0;
}

int main() {
    assert(Sensor::live_objects == 0);
    {
        auto sensors = make_sensors();
        assert(sensors.size() == 2);
        assert(dynamic_cast<Camera*>(sensors[0].get()) != nullptr);
        assert(dynamic_cast<Lidar*>(sensors[1].get()) != nullptr);
        assert(sensors[0]->name() == "camera");
        assert(sensors[1]->name() == "lidar");
        assert(total_rate(sensors) == 40.0);
        assert(Sensor::live_objects == 2);
    }
    assert(Sensor::live_objects == 0);
    std::cout << "ALL TESTS PASSED\n";
}
