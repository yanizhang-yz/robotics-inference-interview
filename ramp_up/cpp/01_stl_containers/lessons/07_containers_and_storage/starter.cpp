#include <cassert>
#include <cstddef>
#include <cstdint>
#include <iostream>
#include <vector>

std::vector<std::uint8_t> make_gray_frame(
    std::size_t width,
    std::size_t height,
    std::uint8_t fill
) {
    // Exercise: allocate one grayscale pixel for each frame position.
    (void)width;
    (void)height;
    (void)fill;
    return {};
}

long long checksum(const std::vector<std::uint8_t>& pixels) {
    // Exercise: add every pixel value without modifying the frame.
    (void)pixels;
    return 0;
}

int main() {
    const auto frame = make_gray_frame(4, 3, 7);
    assert(frame.size() == 12);
    assert(frame.front() == 7 && frame.back() == 7);
    assert(checksum(frame) == 84);
    assert(frame.data() + frame.size() == &frame.back() + 1);
    std::cout << "ALL TESTS PASSED\n";
}
