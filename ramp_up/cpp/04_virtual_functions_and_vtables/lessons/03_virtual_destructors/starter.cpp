// DRILL BRIEF
// Concept: A virtual base destructor completes polymorphic cleanup in derived-to-base order.
// Scenario: Destroy a Camera safely through a unique_ptr<Sensor>.
// Implement: Both logging destructors and the payload's release counter.
// Behavior: Log Camera then Sensor and release the Camera payload exactly once.
// Interview focus: Recognize why polymorphic base classes need virtual destructors.
// Tests: main performs only safe unique_ptr-based deletion and checks cleanup evidence.
// Run: PRACTICE=1 uv run pytest ramp_up/cpp/04_virtual_functions_and_vtables/lessons/03_virtual_destructors -q
// Done when: The test passes and the program prints ALL TESTS PASSED.

#include <cassert>
#include <iostream>
#include <memory>
#include <string>
#include <vector>

class Sensor {
public:
    explicit Sensor(std::vector<std::string>& log) : log_(log) {}
    virtual ~Sensor() {
        // TODO: log base cleanup.
    }

protected:
    std::vector<std::string>& log_;
};

class ReleasedPayload {
public:
    explicit ReleasedPayload(int& releases) : releases_(releases) {}
    ~ReleasedPayload() {
        // TODO: record release.
    }

private:
    int& releases_;
};

class Camera final : public Sensor {
public:
    Camera(std::vector<std::string>& log, int& releases)
        : Sensor(log), payload_(releases) {}
    ~Camera() override {
        // TODO: log derived cleanup.
    }

private:
    ReleasedPayload payload_;
};

int main() {
    std::vector<std::string> log;
    int released_payloads = 0;
    {
        std::unique_ptr<Sensor> sensor =
            std::make_unique<Camera>(log, released_payloads);
    }
    assert((log == std::vector<std::string>{"Camera", "Sensor"}));
    assert(released_payloads == 1);
    std::cout << "ALL TESTS PASSED\n";
}
