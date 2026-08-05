#include <cassert>
#include <cstdint>
#include <iostream>
#include <vector>

struct Frame {
    int id;
    std::vector<std::uint8_t> pixels;
};

Frame* find_frame(std::vector<Frame>& frames, int id) {
    for (Frame& frame : frames) {
        if (frame.id == id) return &frame;
    }
    return nullptr;
}

long long checksum(const Frame& frame) {
    long long total = 0;
    for (std::uint8_t pixel : frame.pixels) total += pixel;
    return total;
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
