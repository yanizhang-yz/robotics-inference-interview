// DRILL BRIEF
// Concept: Combine dynamic dispatch, override checking, polymorphic cleanup, and static dispatch.
// Scenario: Complete a mixed camera/lidar polling backend and diagnose forgotten virtual dispatch.
// Implement: Sensor cleanup, both backends, pollAll, describe, and brokenDispatchDemo.
// Behavior: Poll mixed sensors, describe through references, clean up in order, and expose static dispatch.
// Example: camera/lidar poll as camera=30 and lidar=10. Edge: base-owned Camera destruction logs Camera then Sensor, while non-virtual id uses the base result.
// Interview focus: Explain which type selects each call and why base destruction must be virtual.
// Tests: main checks the existing dispatch, formatting, destruction, and broken-dispatch assertions.
// Run: PRACTICE=1 uv run pytest ramp_up/cpp/04_virtual_functions_and_vtables/test_solution.py -q
// Done when: The test passes and the program prints ALL TESTS PASSED.
// 04_virtual_functions_and_vtables — YOUR attempt. Fill in the TODO bodies, then run:
//   PRACTICE=1 uv run pytest ramp_up/cpp/04_virtual_functions_and_vtables/test_solution.py -v
// Or compile and run directly:
//   clang++ -std=c++20 -Wall -Wextra -Werror=return-type -o /tmp/vtables starter.cpp && /tmp/vtables
// The stubs compile as-is but fail main()'s asserts until you implement them.
// You're done when the last line printed is: ALL TESTS PASSED

#include <cassert>
#include <iostream>
#include <memory>
#include <sstream>
#include <string>
#include <vector>

// Provided test log for observable destruction order.
std::vector<std::string>& destructionLog() {
    static std::vector<std::string> log;
    return log;
}

// Sensor is the common interface. Its destruction must remain observable and
// safe through an owning Sensor pointer.
class Sensor {
public:
    virtual ~Sensor() { /* TODO: log "Sensor" */ }
    virtual std::string name() const = 0;
    virtual double read() = 0;
};

// Camera and Lidar provide the names, rates, and destruction events asserted in main.
class Camera : public Sensor {
public:
    ~Camera() override { /* TODO: log "Camera" */ }
    std::string name() const override {
        // TODO: implement
        return "";
    }
    double read() override {
        // TODO: implement
        return 0.0;
    }
};

class Lidar : public Sensor {
public:
    ~Lidar() override { /* TODO: log "Lidar" */ }
    std::string name() const override {
        // TODO: implement
        return "";
    }
    double read() override {
        // TODO: implement
        return 0.0;
    }
};

// Format one result per mixed Sensor owner in input order, without type tests.
std::vector<std::string> pollAll(const std::vector<std::unique_ptr<Sensor>>& sensors) {
    // TODO: implement
    return {};
}

// Describe the dynamic sensor behind a non-owning const interface reference.
std::string describe(const Sensor& sensor) {
    // TODO: implement the asserted description format.
    return "";
}

// These provided types intentionally omit virtual dispatch; do not change them.
struct BrokenBase {
    std::string id() const { return "BrokenBase"; }
};
struct BrokenDerived : BrokenBase {
    std::string id() const { return "BrokenDerived"; }
};

// Return the identifier selected through a BrokenBase pointer to a derived object.
std::string brokenDispatchDemo() {
    // TODO: implement
    return "";
}

int main() {
    // Dynamic dispatch and concrete outputs.
    {
        std::unique_ptr<Sensor> cam = std::make_unique<Camera>();
        std::unique_ptr<Sensor> lid = std::make_unique<Lidar>();
        assert(cam->name() == "camera");
        assert(lid->name() == "lidar");
        assert(cam->read() == 30.0);
        assert(lid->read() == 10.0);
    }

    // Mixed-owner polling.
    {
        std::vector<std::unique_ptr<Sensor>> sensors;
        sensors.push_back(std::make_unique<Camera>());
        sensors.push_back(std::make_unique<Lidar>());
        sensors.push_back(std::make_unique<Camera>());
        auto lines = pollAll(sensors);
        assert((lines ==
                std::vector<std::string>{"camera=30", "lidar=10", "camera=30"}));
    }

    // Description through a borrowed interface.
    {
        Camera cam;
        Lidar lid;
        assert(describe(cam) == "Sensor[camera]");
        assert(describe(lid) == "Sensor[lidar]");
        const Sensor& asBase = cam;
        assert(describe(asBase) == "Sensor[camera]");
    }

    // Destruction through an interface owner.
    {
        destructionLog().clear();
        {
            std::unique_ptr<Sensor> s = std::make_unique<Camera>();
        }
        assert((destructionLog() == std::vector<std::string>{"Camera", "Sensor"}));

        destructionLog().clear();
        {
            std::unique_ptr<Sensor> s = std::make_unique<Lidar>();
        }
        assert((destructionLog() == std::vector<std::string>{"Lidar", "Sensor"}));
    }

    // Static-dispatch contrast.
    assert(brokenDispatchDemo() == "BrokenBase");

    std::cout << "ALL TESTS PASSED" << std::endl;
    return 0;
}
