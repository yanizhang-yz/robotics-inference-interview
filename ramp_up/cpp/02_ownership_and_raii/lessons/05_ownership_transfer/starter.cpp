// DRILL BRIEF
// Concept: Moving a unique_ptr transfers cleanup responsibility to one new owner.
// Scenario: Relay a captured frame to inference, then consume it.
// Implement: relay_frame and consume_frame using their by-value owner parameters.
// Behavior: Moves empty the source, preserve the pointee address, and return checksum 15.
// Interview focus: Trace ownership at each std::move and explain moved-from emptiness.
// Tests: main checks the source/receiver states, address identity, and consumption result.
// Run: PRACTICE=1 uv run pytest ramp_up/cpp/02_ownership_and_raii/lessons/05_ownership_transfer -q
// Done when: The test run passes and the program prints ALL TESTS PASSED.

#include <cassert>
#include <cstdint>
#include <iostream>
#include <memory>
#include <utility>
#include <vector>

struct Frame {
    int id;
    std::vector<std::uint8_t> pixels;
};

long long checksum(const Frame& frame) {
    long long total = 0;
    for (std::uint8_t pixel : frame.pixels) total += pixel;
    return total;
}

std::unique_ptr<Frame> relay_frame(std::unique_ptr<Frame> frame) {
    // Exercise: pass this same owner onward without allocating another Frame.
    (void)frame;
    return nullptr;
}

long long consume_frame(std::unique_ptr<Frame> frame) {
    // Exercise: read the owned frame; it will clean up when this call ends.
    (void)frame;
    return 0;
}

int main() {
    auto captured = std::make_unique<Frame>(Frame{7, {4, 5, 6}});
    Frame* original_address = captured.get();
    auto relayed = relay_frame(std::move(captured));
    assert(captured == nullptr);
    assert(relayed.get() == original_address);
    assert(consume_frame(std::move(relayed)) == 15);
    assert(relayed == nullptr);
    std::cout << "ALL TESTS PASSED\n";
}
