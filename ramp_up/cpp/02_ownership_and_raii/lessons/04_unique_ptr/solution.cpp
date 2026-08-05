#include <cassert>
#include <cstddef>
#include <cstdint>
#include <iostream>
#include <memory>
#include <vector>

struct Frame {
    static int live_count;
    Frame(std::size_t frame_width, std::size_t frame_height, std::uint8_t fill)
        : width(frame_width), height(frame_height), pixels(width * height, fill) {
        ++live_count;
    }
    ~Frame() { --live_count; }
    std::size_t width;
    std::size_t height;
    std::vector<std::uint8_t> pixels;
};

int Frame::live_count = 0;

long long checksum(const Frame& frame) {
    long long total = 0;
    for (std::uint8_t pixel : frame.pixels) total += pixel;
    return total;
}

std::unique_ptr<Frame> make_frame(
    std::size_t width, std::size_t height, std::uint8_t fill
) {
    return std::make_unique<Frame>(width, height, fill);
}

int main() {
    assert(Frame::live_count == 0);
    {
        auto frame = make_frame(3, 2, 7);
        assert(frame != nullptr);
        assert(frame->width == 3);
        assert(frame->height == 2);
        assert(frame->pixels.size() == 6);
        assert(checksum(*frame) == 42);
        assert(Frame::live_count == 1);
    }
    assert(Frame::live_count == 0);
    std::cout << "ALL TESTS PASSED\n";
}
