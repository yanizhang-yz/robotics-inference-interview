#include <cassert>
#include <cstdint>
#include <iostream>
#include <vector>

class TrackedFrame {
public:
    inline static int copies = 0;

    TrackedFrame(int width, int height, std::uint8_t fill)
        : pixels_(static_cast<std::size_t>(width) * height, fill) {}

    TrackedFrame(const TrackedFrame& other) : pixels_(other.pixels_) { ++copies; }

    long long checksum() const {
        long long total = 0;
        for (std::uint8_t pixel : pixels_) total += pixel;
        return total;
    }

    static void reset_counters() { copies = 0; }

private:
    std::vector<std::uint8_t> pixels_;
};

long long inspect_by_value(TrackedFrame frame) {
    return frame.checksum();
}

long long inspect_by_const_ref(const TrackedFrame& frame) {
    return frame.checksum();
}

int main() {
    TrackedFrame frame(3, 2, 5);

    TrackedFrame::reset_counters();
    const long long by_value = inspect_by_value(frame);
    assert(by_value == 30);
    assert(TrackedFrame::copies == 1);

    TrackedFrame::reset_counters();
    const long long by_ref = inspect_by_const_ref(frame);
    assert(by_ref == by_value);
    assert(TrackedFrame::copies == 0);

    std::cout << "ALL TESTS PASSED\n";
}
