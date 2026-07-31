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
