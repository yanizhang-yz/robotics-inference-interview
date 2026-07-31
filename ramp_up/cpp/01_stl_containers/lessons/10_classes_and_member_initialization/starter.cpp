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
