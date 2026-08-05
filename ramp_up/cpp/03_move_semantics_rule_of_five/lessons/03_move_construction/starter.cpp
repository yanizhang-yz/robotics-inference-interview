// DRILL BRIEF
// Concept: A move constructor transfers an owning handle instead of copying pixels.
// Scenario: Hand a captured pixel buffer into a newly constructed inference input.
// Implement: PixelBuffer(PixelBuffer&&) noexcept and establish an empty source.
// Behavior: The destination preserves the allocation address; the source becomes 0x0.
// Interview focus: Trace ownership and distinguish handle movement from byte copying.
// Tests: main checks dimensions, size, address identity, and moved-from state.
// Run: PRACTICE=1 uv run pytest ramp_up/cpp/03_move_semantics_rule_of_five/lessons/03_move_construction -q
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

    PixelBuffer(PixelBuffer&& other) noexcept {
        // TODO: transfer the owning state and empty other.
        (void)other;
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
    PixelBuffer captured(4, 3, 7);
    const std::uint8_t* captured_address = captured.data();
    PixelBuffer input(std::move(captured));
    assert(input.width() == 4 && input.height() == 3 && input.size() == 12);
    assert(input.data() == captured_address);
    assert(captured.width() == 0 && captured.height() == 0 && captured.size() == 0);
    std::cout << "ALL TESTS PASSED\n";
}
