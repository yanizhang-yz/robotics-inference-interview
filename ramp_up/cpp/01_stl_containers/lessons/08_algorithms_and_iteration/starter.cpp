// DRILL BRIEF
// Concept: Filter a sequence by a cutoff, then sort the selected IDs.
// Scenario: Produce fresh camera frame IDs for an inference pipeline.
// Implement: fresh_frame_ids.
// Behavior: frames {3@30, 1@10, 2@20} return {2, 3} for cutoff 20 and an empty vector for cutoff 40.
// Interview focus: Filter then sort frame IDs in fresh_frame_ids.
// Tests: main asserts the sorted eligible IDs and the no-match case.
// Run: PRACTICE=1 uv run pytest ramp_up/cpp/01_stl_containers/lessons/08_algorithms_and_iteration -q
// Done when: The test run passes and the program prints ALL TESTS PASSED.

#include <algorithm>
#include <cassert>
#include <cstdint>
#include <iostream>
#include <vector>

struct CameraFrame {
    int frame_id;
    std::int64_t timestamp_ns;
};

std::vector<int> fresh_frame_ids(
    const std::vector<CameraFrame>& frames,
    std::int64_t cutoff_ns
) {
    // Exercise: select each frame at or after the cutoff, then sort its ID.
    (void)frames;
    (void)cutoff_ns;
    return {};
}

int main() {
    const std::vector<CameraFrame> frames{{3, 30}, {1, 10}, {2, 20}};
    assert((fresh_frame_ids(frames, 20) == std::vector<int>{2, 3}));
    assert(fresh_frame_ids(frames, 40).empty());
    std::cout << "ALL TESTS PASSED\n";
}
