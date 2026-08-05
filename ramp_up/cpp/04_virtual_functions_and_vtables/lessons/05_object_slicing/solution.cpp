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
    std::string kind() const override { return "camera"; }
};

std::string kind_by_value(SensorRecord sensor) {
    return sensor.kind();
}

std::string kind_by_reference(const SensorRecord& sensor) {
    return sensor.kind();
}

int main() {
    CameraRecord camera;
    assert(kind_by_value(camera) == "sensor");
    assert(kind_by_reference(camera) == "camera");
    std::cout << "ALL TESTS PASSED\n";
}
