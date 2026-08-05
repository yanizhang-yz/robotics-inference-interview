#include <cassert>
#include <iostream>
#include <stdexcept>

class ScopedDevice {
public:
    explicit ScopedDevice(bool& active) : active_(active) { active_ = true; }
    ~ScopedDevice() { active_ = false; }
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
