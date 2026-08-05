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
    batch.source = std::move(source);
    return batch;
}

int main() {
    FrameBatch captured{"front-camera", {{1, {3, 4, 5}}}};
    FrameBatch labeled = relabel_copy(captured, "training-set");
    assert(labeled.source == "training-set");
    assert(captured.source == "front-camera");
    labeled.frames[0].pixels[0] = 99;
    assert(captured.frames[0].pixels[0] == 3);

    const std::uint8_t* original_address = captured.frames[0].pixels.data();
    FrameBatch moved = std::move(captured);
    assert(moved.frames[0].pixels.data() == original_address);
    assert(captured.frames.empty());

    std::cout << "ALL TESTS PASSED\n";
}
