#include <cassert>
#include <cstddef>
#include <cstdint>
#include <iostream>
#include <vector>

struct Frame {
    std::size_t width;
    std::size_t height;
    std::vector<std::uint8_t> pixels;
};

std::size_t frame_payload_bytes(const Frame& frame) {
    return frame.pixels.size() * sizeof(std::uint8_t);
}

bool pixels_are_contiguous(const Frame& frame) {
    for (std::size_t i = 1; i < frame.pixels.size(); ++i) {
        if (&frame.pixels[i] != &frame.pixels[i - 1] + 1) return false;
    }
    return true;
}

int main() {
    const Frame frame{4, 3, std::vector<std::uint8_t>(12, 7)};
    assert(frame_payload_bytes(frame) == frame.width * frame.height);
    assert(pixels_are_contiguous(frame));
    assert(&frame.pixels[1] == &frame.pixels[0] + 1);
    assert(sizeof(Frame) != frame_payload_bytes(frame));
    std::cout << "ALL TESTS PASSED\n";
}
