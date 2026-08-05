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

    PixelBuffer& operator=(PixelBuffer&& other) noexcept {
        if (this != &other) {
            width_ = other.width_;
            height_ = other.height_;
            data_ = std::move(other.data_);
            other.width_ = 0;
            other.height_ = 0;
            other.data_.clear();
        }
        return *this;
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
    PixelBuffer source(3, 2, 9);
    PixelBuffer target(1, 1, 1);
    const std::uint8_t* source_address = source.data();
    target = std::move(source);

    assert(target.width() == 3 && target.height() == 2 && target.size() == 6);
    assert(target.data() == source_address);
    assert(source.width() == 0 && source.height() == 0 && source.size() == 0);

    target = std::move(target);
    assert(target.width() == 3 && target.height() == 2 && target.size() == 6);

    source = PixelBuffer(2, 2, 4);
    assert(source.width() == 2 && source.height() == 2 && source.size() == 4);

    std::cout << "ALL TESTS PASSED\n";
}
