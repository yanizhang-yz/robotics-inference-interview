// DRILL BRIEF
// Concept: A container owns its elements; references and raw pointers borrow them.
// Scenario: Find a frame in owned pipeline storage and inspect its pixels.
// Implement: find_frame and checksum without allocating, deleting, or mutating.
// Behavior: Return the matching element's address, nullptr when absent, and pixel sum.
// Example: finding id 2 aliases its vector element and checksum sums its pixels. Edge: a missing id returns nullptr without mutation.
// Interview focus: Name the owner, borrower, null case, and vector invalidation boundary.
// Tests: main checks alias identity, absence, checksum, and unchanged owned storage.
// Run: PRACTICE=1 uv run pytest ramp_up/cpp/02_ownership_and_raii/lessons/03_owners_and_borrowers -q
// Done when: The test run passes and the program prints ALL TESTS PASSED.

#include <cassert>
#include <cstdint>
#include <iostream>
#include <vector>

struct Frame {
    int id;
    std::vector<std::uint8_t> pixels;
};

Frame* find_frame(std::vector<Frame>& frames, int id) {
    // Exercise: return a pointer to the matching vector element, or nullptr.
    (void)frames;
    (void)id;
    return nullptr;
}

long long checksum(const Frame& frame) {
    // Exercise: read through the const reference and sum the pixels.
    (void)frame;
    return 0;
}

int main() {
    std::vector<Frame> frames{{10, {1, 2}}, {20, {3, 4, 5}}};
    const auto before = frames;
    Frame* match = find_frame(frames, 20);
    assert(match == &frames[1]);
    assert(find_frame(frames, 99) == nullptr);
    assert(checksum(frames[1]) == 12);
    assert(frames.size() == 2);
    assert(frames[0].pixels == before[0].pixels);
    assert(frames[1].pixels == before[1].pixels);
    std::cout << "ALL TESTS PASSED\n";
}
