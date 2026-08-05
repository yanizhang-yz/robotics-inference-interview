// DRILL BRIEF
// Concept: Initialize class members into a valid, internally consistent frame.
// Scenario: Construct a filled camera frame for an image-processing stage.
// Implement: CameraFrame constructor.
// Behavior: CameraFrame(4, 3, 7) has width 4, height 3, pixel_count 12, and checksum 84.
// Interview focus: Build a valid camera frame in its constructor.
// Tests: main checks dimensions, derived pixel count, and the filled-frame checksum.
// Run: PRACTICE=1 uv run pytest ramp_up/cpp/01_stl_containers/lessons/10_classes_and_member_initialization -q
// Done when: The test run passes and the program prints ALL TESTS PASSED.

#include <cassert>
#include <cstddef>
#include <cstdint>
#include <iostream>
#include <vector>

class CameraFrame {
public:
    CameraFrame(std::size_t width, std::size_t height, std::uint8_t fill)
        : width_(0),
          height_(0),
          pixels_() {
        (void)width;
        (void)height;
        (void)fill;
    }

    std::size_t width() const { return width_; }
    std::size_t height() const { return height_; }
    std::size_t pixel_count() const { return pixels_.size(); }

    long long checksum() const {
        long long total = 0;
        for (std::uint8_t pixel : pixels_) {
            total += pixel;
        }
        return total;
    }

private:
    std::size_t width_;
    std::size_t height_;
    std::vector<std::uint8_t> pixels_;
};

int main() {
    const CameraFrame frame(4, 3, 7);
    assert(frame.width() == 4);
    assert(frame.height() == 3);
    assert(frame.pixel_count() == 12);
    assert(frame.checksum() == 84);
    std::cout << "ALL TESTS PASSED\n";
}
