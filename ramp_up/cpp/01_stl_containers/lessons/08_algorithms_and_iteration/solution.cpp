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
    std::vector<int> ids;
    for (const CameraFrame& frame : frames) {
        if (frame.timestamp_ns >= cutoff_ns) {
            ids.push_back(frame.frame_id);
        }
    }
    std::sort(ids.begin(), ids.end());
    return ids;
}

int main() {
    const std::vector<CameraFrame> frames{{3, 30}, {1, 10}, {2, 20}};
    assert((fresh_frame_ids(frames, 20) == std::vector<int>{2, 3}));
    assert(fresh_frame_ids(frames, 40).empty());
    std::cout << "ALL TESTS PASSED\n";
}
