// DRILL BRIEF
// Concept: Separate an object's member storage from a vector's payload storage.
// Scenario: Account for the bytes in a grayscale inference frame.
// Implement: frame_payload_bytes and pixels_are_contiguous.
// Behavior: Report width-times-height payload bytes and verify adjacent pixels.
// Example: a 3x2 frame reports six payload bytes and contiguous addresses. Edge: an empty frame reports zero payload bytes.
// Interview focus: Explain why sizeof(Frame) is not the image payload size.
// Tests: main checks payload bytes, adjacent addresses, and object versus payload.
// Run: PRACTICE=1 uv run pytest ramp_up/cpp/05_memory_layout_and_cache/lessons/01_storage_and_object_layout -q
// Done when: The test passes and the program prints ALL TESTS PASSED.

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
    // TODO: return the bytes owned by the pixel vector's allocation.
    (void)frame;
    return 0;
}

bool pixels_are_contiguous(const Frame& frame) {
    // TODO: compare each pixel address with the address after its predecessor.
    (void)frame;
    return false;
}

int main() {
    const Frame frame{4, 3, std::vector<std::uint8_t>(12, 7)};
    assert(frame_payload_bytes(frame) == frame.width * frame.height);
    assert(pixels_are_contiguous(frame));
    assert(&frame.pixels[1] == &frame.pixels[0] + 1);
    assert(sizeof(Frame) != frame_payload_bytes(frame));
    std::cout << "ALL TESTS PASSED\n";
}
