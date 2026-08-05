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
    std::vector<std::unique_ptr<Sensor>> sensors;
    sensors.push_back(std::make_unique<Camera>());
    sensors.push_back(std::make_unique<Lidar>());
    return sensors;
}

double total_rate(const std::vector<std::unique_ptr<Sensor>>& sensors) {
    double total = 0.0;
    for (const auto& sensor : sensors) total += sensor->rate_hz();
    return total;
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
