// DRILL BRIEF
// Concept: Pass a record by value and return a revised value without mutating the original.
// Scenario: Assign a new frame ID while preserving camera metadata.
// Implement: with_frame_id.
// Behavior: original {7, "wrist-camera"} remains frame_id 7; with_frame_id(original, 8) returns frame_id 8 with source "wrist-camera".
// Interview focus: Return revised metadata by value from with_frame_id.
// Tests: main asserts the original record, revised ID, and preserved source.
// Run: PRACTICE=1 uv run pytest ramp_up/cpp/01_stl_containers/lessons/03_values_and_copies -q
// Done when: The test run passes and the program prints ALL TESTS PASSED.

#include <cassert>
#include <iostream>
#include <string>

struct FrameMetadata {
    int frame_id;
    std::string source;
};

FrameMetadata with_frame_id(FrameMetadata metadata, int frame_id) {
    // Exercise: produce revised metadata without changing the original record.
    (void)frame_id;
    return metadata;
}

int main() {
    FrameMetadata original{7, "wrist-camera"};
    FrameMetadata updated = with_frame_id(original, 8);
    assert(original.frame_id == 7);
    assert(updated.frame_id == 8);
    assert(updated.source == "wrist-camera");
    std::cout << "ALL TESTS PASSED\n";
}
