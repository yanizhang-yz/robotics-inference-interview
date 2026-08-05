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
    std::string name() const override { return "camera"; }
    double rate_hz() const override { return 30.0; }
};

class Lidar final : public Sensor {
public:
    std::string name() const override { return "lidar"; }
    double rate_hz() const override { return 10.0; }
};

std::vector<std::string> poll(const std::vector<const Sensor*>& sensors) {
    std::vector<std::string> readings;
    for (const Sensor* sensor : sensors) {
        readings.push_back(sensor->name() + "=" +
                           std::to_string(sensor->rate_hz()));
    }
    return readings;
}

int main() {
    Camera camera;
    Lidar lidar;
    const std::vector<const Sensor*> sensors{&camera, &lidar};
    assert((poll(sensors) ==
            std::vector<std::string>{"camera=30.000000", "lidar=10.000000"}));
    std::cout << "ALL TESTS PASSED\n";
}
