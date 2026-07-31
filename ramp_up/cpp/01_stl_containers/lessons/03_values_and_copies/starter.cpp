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
