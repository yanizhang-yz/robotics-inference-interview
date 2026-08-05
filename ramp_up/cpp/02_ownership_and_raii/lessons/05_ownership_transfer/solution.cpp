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
    return frame;
}

long long consume_frame(std::unique_ptr<Frame> frame) {
    return checksum(*frame);
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
