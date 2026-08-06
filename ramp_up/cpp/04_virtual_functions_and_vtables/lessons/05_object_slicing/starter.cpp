// DRILL BRIEF
// Concept: Passing a derived object by base value slices away the derived part.
// Scenario: Compare a legacy by-value record API with a const-reference repair.
// Implement: CameraRecord::kind, kind_by_value, and kind_by_reference.
// Behavior: The sliced call says sensor; the borrowed call dispatches to camera.
// Example: kind_by_value(camera) is sensor and kind_by_reference(camera) is camera. Edge: the base-value boundary intentionally discards derived state.
// Interview focus: Diagnose lost polymorphism at a by-value base-class boundary.
// Tests: main passes the same CameraRecord through both signatures.
// Run: PRACTICE=1 uv run pytest ramp_up/cpp/04_virtual_functions_and_vtables/lessons/05_object_slicing -q
// Done when: The test passes and the program prints ALL TESTS PASSED.

#include <cassert>
#include <iostream>
#include <string>

class SensorRecord {
public:
    virtual ~SensorRecord() = default;
    virtual std::string kind() const { return "sensor"; }
};

class CameraRecord final : public SensorRecord {
public:
    std::string kind() const override {
        // TODO: identify the derived record.
        return "";
    }
};

std::string kind_by_value(SensorRecord sensor) {
    // TODO: ask the copied base object for its kind.
    (void)sensor;
    return "";
}

std::string kind_by_reference(const SensorRecord& sensor) {
    // TODO: ask the borrowed polymorphic object for its kind.
    (void)sensor;
    return "";
}

int main() {
    CameraRecord camera;
    assert(kind_by_value(camera) == "sensor");
    assert(kind_by_reference(camera) == "camera");
    std::cout << "ALL TESTS PASSED\n";
}
