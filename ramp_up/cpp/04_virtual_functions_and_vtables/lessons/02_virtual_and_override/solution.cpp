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
    std::string name() const override { return "camera"; }
    double rate_hz() const override { return 30.0; }
};

class Lidar final : public Sensor {
public:
    std::string name() const override { return "lidar"; }
    double rate_hz() const override { return 10.0; }
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
