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
    for (const Sensor& sensor : sensors) {
        if (sensor.name == name) {
            return &sensor;
        }
    }
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
