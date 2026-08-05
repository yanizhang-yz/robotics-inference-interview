#include <cassert>
#include <cstddef>
#include <cstdint>
#include <iostream>
#include <utility>
#include <vector>

class PixelBuffer {
public:
    PixelBuffer(int width, int height, std::uint8_t fill)
        : width_(width),
          height_(height),
          data_(static_cast<std::size_t>(width) * height, fill) {}

    PixelBuffer(PixelBuffer&& other) noexcept
        : width_(other.width_),
          height_(other.height_),
          data_(std::move(other.data_)) {
        other.width_ = 0;
        other.height_ = 0;
    }

    int width() const { return width_; }
    int height() const { return height_; }
    std::size_t size() const { return data_.size(); }
    const std::uint8_t* data() const { return data_.data(); }

private:
    int width_ = 0;
    int height_ = 0;
    std::vector<std::uint8_t> data_;
};

int main() {
    PixelBuffer captured(4, 3, 7);
    const std::uint8_t* captured_address = captured.data();
    PixelBuffer inference_input(std::move(captured));

    assert(inference_input.width() == 4);
    assert(inference_input.height() == 3);
    assert(inference_input.size() == 12);
    assert(inference_input.data() == captured_address);
    assert(captured.width() == 0 && captured.height() == 0);
    assert(captured.size() == 0);

    std::cout << "ALL TESTS PASSED\n";
}
