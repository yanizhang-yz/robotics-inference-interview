// DRILL BRIEF
// Concept: Store a grayscale frame in contiguous vector memory.
// Scenario: Allocate pixels for a camera frame and calculate a checksum.
// Implement: make_gray_frame and checksum.
// Behavior: make_gray_frame(4, 3, 7) has size 12 with front and back 7, checksum 84, and data() + size() == &back() + 1.
// Interview focus: Allocate a frame and compute its checksum.
// Tests: main checks dimensions, fill, checksum, and the contiguous-address relationship.
// Run: PRACTICE=1 uv run pytest ramp_up/cpp/01_stl_containers/lessons/07_containers_and_storage -q
// Done when: The test run passes and the program prints ALL TESTS PASSED.

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
