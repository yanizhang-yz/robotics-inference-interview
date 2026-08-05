// DRILL BRIEF
// Concept: lvalue and rvalue expressions select different reference overloads.
// Scenario: Predict how named and temporary frames enter a pipeline helper.
// Implement: category(const Frame&) and category(Frame&&).
// Behavior: Names report lvalue; temporaries and std::move expressions report rvalue.
// Interview focus: Explain that std::move is a cast and does not itself mutate a frame.
// Tests: main checks overload results and proves a cast-only call preserves the frame.
// Run: PRACTICE=1 uv run pytest ramp_up/cpp/03_move_semantics_rule_of_five/lessons/02_temporaries_and_value_categories -q
// Done when: The test passes and the program prints ALL TESTS PASSED.

#include <cassert>
#include <cstdint>
#include <iostream>
#include <string>
#include <utility>
#include <vector>

struct Frame { std::vector<std::uint8_t> pixels; };

std::string category(const Frame& frame) {
    // TODO: classify this overload.
    (void)frame;
    return "unknown";
}

std::string category(Frame&& frame) {
    // TODO: classify this overload.
    (void)frame;
    return "unknown";
}

int main() {
    Frame frame{{1, 2, 3}};
    assert(category(frame) == "lvalue");
    assert(category(Frame{{4, 5}}) == "rvalue");
    const auto* address = frame.pixels.data();
    const std::size_t size = frame.pixels.size();
    assert(category(std::move(frame)) == "rvalue");
    assert(frame.pixels.data() == address && frame.pixels.size() == size);
    std::cout << "ALL TESTS PASSED\n";
}
