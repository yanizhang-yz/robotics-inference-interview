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
    return std::vector<std::uint8_t>(width * height, fill);
}

long long checksum(const std::vector<std::uint8_t>& pixels) {
    long long total = 0;
    for (std::uint8_t pixel : pixels) {
        total += pixel;
    }
    return total;
}

int main() {
    const auto frame = make_gray_frame(4, 3, 7);
    assert(frame.size() == 12);
    assert(frame.front() == 7 && frame.back() == 7);
    assert(checksum(frame) == 84);
    assert(frame.data() + frame.size() == &frame.back() + 1);
    std::cout << "ALL TESTS PASSED\n";
}
