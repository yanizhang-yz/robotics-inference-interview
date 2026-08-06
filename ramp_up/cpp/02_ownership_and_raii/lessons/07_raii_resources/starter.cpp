// DRILL BRIEF
// Concept: RAII puts release in a destructor so every scope exit cleans up.
// Scenario: Keep a sensor device active only during one read operation.
// Implement: ScopedDevice's constructor/destructor state changes and read result.
// Behavior: active is true during reads and false after normal or exceptional exit.
// Example: a normal read returns 42 while the device is active. Edge: a thrown read still leaves active false.
// Interview focus: Explain exception safety and why explicit paired cleanup is fragile.
// Tests: main checks normal return, a thrown runtime_error, and both cleanup paths.
// Run: PRACTICE=1 uv run pytest ramp_up/cpp/02_ownership_and_raii/lessons/07_raii_resources -q
// Done when: The test run passes and the program prints ALL TESTS PASSED.

#include <cassert>
#include <iostream>
#include <stdexcept>

class ScopedDevice {
public:
    explicit ScopedDevice(bool& active) : active_(active) {
        // Exercise: acquire the device by setting the borrowed flag.
    }
    ~ScopedDevice() {
        // Exercise: release the device on every scope-exit path.
    }
    ScopedDevice(const ScopedDevice&) = delete;
    ScopedDevice& operator=(const ScopedDevice&) = delete;

private:
    bool& active_;
};

int read_with_device(bool& active, bool throw_after_open) {
    ScopedDevice device(active);
    if (throw_after_open) throw std::runtime_error("sensor read failed");
    return active ? 42 : 0;
}

int main() {
    bool active = false;
    assert(read_with_device(active, false) == 42);
    assert(!active);
    try {
        read_with_device(active, true);
        assert(false);
    } catch (const std::runtime_error&) {
        assert(!active);
    }
    std::cout << "ALL TESTS PASSED\n";
}
