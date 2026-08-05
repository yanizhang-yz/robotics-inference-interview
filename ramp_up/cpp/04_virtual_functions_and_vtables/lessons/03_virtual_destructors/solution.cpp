#include <cassert>
#include <iostream>
#include <memory>
#include <string>
#include <vector>

class Sensor {
public:
    explicit Sensor(std::vector<std::string>& log) : log_(log) {}
    virtual ~Sensor() { log_.push_back("Sensor"); }

protected:
    std::vector<std::string>& log_;
};

class ReleasedPayload {
public:
    explicit ReleasedPayload(int& releases) : releases_(releases) {}
    ~ReleasedPayload() { ++releases_; }

private:
    int& releases_;
};

class Camera final : public Sensor {
public:
    Camera(std::vector<std::string>& log, int& releases)
        : Sensor(log), payload_(releases) {}
    ~Camera() override { log_.push_back("Camera"); }

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
