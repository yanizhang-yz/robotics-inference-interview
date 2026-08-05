// DRILL BRIEF
// Concept: Resource-owning standard members supply correct special members automatically.
// Scenario: Copy a frame batch to relabel it, then move a batch into its next stage.
// Implement: relabel_copy without declaring any FrameBatch special members.
// Behavior: The relabeled copy is independent and a moved batch preserves allocation identity.
// Interview focus: Recognize when writing no destructor/copy/move operations is safest.
// Tests: main checks copy independence, source labels, and moved buffer address identity.
// Run: PRACTICE=1 uv run pytest ramp_up/cpp/03_move_semantics_rule_of_five/lessons/07_rule_of_zero -q
// Done when: The test passes and the program prints ALL TESTS PASSED.

#include <cassert>
#include <cstdint>
#include <iostream>
#include <string>
#include <utility>
#include <vector>

struct Frame {
    int id;
    std::vector<std::uint8_t> pixels;
};

struct FrameBatch {
    std::string source;
    std::vector<Frame> frames;
};

FrameBatch relabel_copy(FrameBatch batch, std::string source) {
    // TODO: update the local copy's source, then return it.
    (void)source;
    return batch;
}

int main() {
    FrameBatch captured{"front-camera", {{1, {3, 4, 5}}}};
    FrameBatch labeled = relabel_copy(captured, "training-set");
    assert(labeled.source == "training-set" && captured.source == "front-camera");
    labeled.frames[0].pixels[0] = 99;
    assert(captured.frames[0].pixels[0] == 3);

    const std::uint8_t* original_address = captured.frames[0].pixels.data();
    FrameBatch moved = std::move(captured);
    assert(moved.frames[0].pixels.data() == original_address);
    assert(captured.frames.empty());
    std::cout << "ALL TESTS PASSED\n";
}
