#include <cassert>
#include <cstdint>
#include <iostream>
#include <string>
#include <utility>
#include <vector>

struct Frame {
    std::vector<std::uint8_t> pixels;
};

std::string category(const Frame& frame) {
    (void)frame;
    return "lvalue";
}

std::string category(Frame&& frame) {
    (void)frame;
    return "rvalue";
}

int main() {
    Frame frame{{1, 2, 3}};
    assert(category(frame) == "lvalue");
    assert(category(Frame{{4, 5}}) == "rvalue");

    const auto* address = frame.pixels.data();
    const std::size_t size = frame.pixels.size();
    assert(category(std::move(frame)) == "rvalue");
    assert(frame.pixels.data() == address);
    assert(frame.pixels.size() == size);

    std::cout << "ALL TESTS PASSED\n";
}
