// DRILL BRIEF
// Concept: A C++17 returned prvalue is constructed directly in its destination.
// Scenario: Build a frame-returning factory without output parameters or extra moves.
// Implement: make_frame(int, int) by returning the requested Frame value.
// Behavior: Dimensions are correct and copy/move counters both remain zero.
// Interview focus: Explain guaranteed copy elision and why return std::move(local) can hurt.
// Tests: main resets counters immediately before the factory call and checks them.
// Run: PRACTICE=1 uv run pytest ramp_up/cpp/03_move_semantics_rule_of_five/lessons/05_copy_elision -q
// Done when: The test passes and the program prints ALL TESTS PASSED.

#include <cassert>
#include <cstddef>
#include <cstdint>
#include <iostream>
#include <utility>
#include <vector>

class Frame {
public:
    inline static int copies = 0;
    inline static int moves = 0;
    Frame(int width, int height)
        : width_(width), height_(height),
          pixels_(static_cast<std::size_t>(width) * height) {}
    Frame(const Frame& other)
        : width_(other.width_), height_(other.height_), pixels_(other.pixels_) {
        ++copies;
    }
    Frame(Frame&& other) noexcept
        : width_(other.width_), height_(other.height_), pixels_(std::move(other.pixels_)) {
        ++moves;
    }
    static void reset_counters() { copies = 0; moves = 0; }
    int width() const { return width_; }
    int height() const { return height_; }
    std::size_t size() const { return pixels_.size(); }

private:
    int width_;
    int height_;
    std::vector<std::uint8_t> pixels_;
};

Frame make_frame(int width, int height) {
    // TODO: return the requested Frame as a prvalue.
    (void)width;
    (void)height;
    return Frame(0, 0);
}

int main() {
    Frame::reset_counters();
    Frame frame = make_frame(5, 4);
    assert(frame.width() == 5 && frame.height() == 4 && frame.size() == 20);
    assert(Frame::copies == 0 && Frame::moves == 0);
    std::cout << "ALL TESTS PASSED\n";
}
