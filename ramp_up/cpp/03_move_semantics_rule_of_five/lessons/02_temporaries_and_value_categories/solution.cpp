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
    return "const-reference overload";
}

std::string category(Frame&& frame) {
    (void)frame;
    return "rvalue-reference overload";
}

int main() {
    Frame frame{{1, 2, 3}};
    assert(category(frame) == "const-reference overload");
    assert(category(Frame{{4, 5}}) == "rvalue-reference overload");

    const auto* address = frame.pixels.data();
    const std::size_t size = frame.pixels.size();
    assert(category(std::move(frame)) == "rvalue-reference overload");
    assert(frame.pixels.data() == address);
    assert(frame.pixels.size() == size);

    std::cout << "ALL TESTS PASSED\n";
}
