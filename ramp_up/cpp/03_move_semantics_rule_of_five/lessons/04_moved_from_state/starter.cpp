// DRILL BRIEF
// Concept: Moved-from objects remain valid; this class strengthens that to empty.
// Scenario: Replace a pipeline buffer, survive self-move, then reuse the old source.
// Implement: PixelBuffer::operator=(PixelBuffer&&) noexcept with a self-move guard.
// Behavior: The target gets the bytes, the source is 0x0, and both remain reusable.
// Example: distinct-source assignment preserves the source address in the target. Edge: self-move and later source reassignment stay valid.
// Interview focus: State the moved-from guarantee and explain self-move handling.
// Tests: main checks address transfer, source emptiness, self-move, and reassignment.
// Run: PRACTICE=1 uv run pytest ramp_up/cpp/03_move_semantics_rule_of_five/lessons/04_moved_from_state -q
// Done when: The test passes and the program prints ALL TESTS PASSED.

#include <cassert>
#include <cstddef>
#include <cstdint>
#include <iostream>
#include <utility>
#include <vector>

class PixelBuffer {
public:
    PixelBuffer(int width, int height, std::uint8_t fill)
        : width_(width), height_(height),
          data_(static_cast<std::size_t>(width) * height, fill) {}

    PixelBuffer& operator=(PixelBuffer&& other) noexcept {
        // TODO: guard self-move, transfer state, and empty a distinct source.
        (void)other;
        return *this;
    }

    int width() const { return width_; }
    int height() const { return height_; }
    std::size_t size() const { return data_.size(); }
    const std::uint8_t* data() const { return data_.data(); }

private:
    int width_ = 0;
    int height_ = 0;
    std::vector<std::uint8_t> data_;
};

int main() {
    PixelBuffer source(3, 2, 9);
    PixelBuffer target(1, 1, 1);
    const std::uint8_t* source_address = source.data();
    target = std::move(source);
    assert(target.width() == 3 && target.height() == 2 && target.size() == 6);
    assert(target.data() == source_address);
    assert(source.width() == 0 && source.height() == 0 && source.size() == 0);

    target = std::move(target);
    assert(target.width() == 3 && target.height() == 2 && target.size() == 6);
    source = PixelBuffer(2, 2, 4);
    assert(source.width() == 2 && source.height() == 2 && source.size() == 4);
    std::cout << "ALL TESTS PASSED\n";
}
