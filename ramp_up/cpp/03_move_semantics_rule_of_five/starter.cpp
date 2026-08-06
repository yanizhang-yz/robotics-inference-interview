// DRILL BRIEF
// Concept: Copy/move selection, moved-from invariants, copy elision, and Rule of Five.
// Scenario: Move grayscale frames through capture, inference, consumption, and swapping.
// Implement: FrameBuffer's copy/move operations, make_frame, consume, and swap_frames.
// Behavior: Copies are deep; moves preserve buffer addresses and empty their sources.
// Example: consuming a 2x2 frame filled with 7 returns 28. Edge: self-assignment and the capstone's empty moved-from invariant remain valid.
// Interview focus: Trace value categories and costs, then explain noexcept and elision.
// Tests: Existing main assertions cover construction, handoffs, factories, sinks, and swap.
// Run: PRACTICE=1 uv run pytest ramp_up/cpp/03_move_semantics_rule_of_five/test_solution.py -q
// Done when: Every existing assertion passes and the program prints ALL TESTS PASSED.

// 03_move_semantics_rule_of_five — YOUR attempt. Fill in the TODO bodies, then run:
//   PRACTICE=1 uv run pytest ramp_up/cpp/03_move_semantics_rule_of_five/test_solution.py -v
// Or compile and run directly:
//   clang++ -std=c++20 -Wall -Wextra -Werror=return-type -o /tmp/moves starter.cpp && /tmp/moves
// The stubs compile as-is but fail main()'s asserts until you implement them.
// You're done when the last line printed is: ALL TESTS PASSED

#include <cassert>
#include <cstddef>
#include <iostream>
#include <utility>
#include <vector>

// FrameBuffer owns width*height grayscale bytes and records copy/move operations.
class FrameBuffer {
public:
    // These counters make the selected special members observable to main().
    inline static int copies_made = 0;
    inline static int moves_made = 0;
    static void reset_counters() { copies_made = 0; moves_made = 0; }

    // Given ordinary construction.
    FrameBuffer(int width, int height, unsigned char fill = 0)
        : width_(width),
          height_(height),
          data_(static_cast<std::size_t>(width) * height, fill) {}

    // Complete the copy/move operations while preserving the ownership invariant.
    ~FrameBuffer() = default;

    // Copies must be independent and increment copies_made.
    FrameBuffer(const FrameBuffer& other) {
        // TODO: implement
    }

    // Copy assignment must also preserve valid self-assignment.
    FrameBuffer& operator=(const FrameBuffer& other) {
        // TODO: implement
        return *this;
    }

    // Moves preserve the allocation address, establish the documented empty
    // source invariant, and increment moves_made.
    FrameBuffer(FrameBuffer&& other) noexcept {
        // TODO: implement
    }

    // Move assignment must additionally preserve valid self-move.
    FrameBuffer& operator=(FrameBuffer&& other) noexcept {
        // TODO: implement
        return *this;
    }

    int width() const { return width_; }
    int height() const { return height_; }
    std::size_t size() const { return data_.size(); }
    const unsigned char* pixels() const { return data_.data(); }
    const std::vector<unsigned char>& bytes() const { return data_; }

private:
    int width_ = 0;
    int height_ = 0;
    std::vector<unsigned char> data_;
};

// Return a zero-filled width-by-height value without observable copy/move work.
FrameBuffer make_frame(int width, int height) {
    // TODO: implement
    return FrameBuffer(0, 0);
}

// Return the checksum of the by-value frame; the call site selects copy or move.
long long consume(FrameBuffer fb) {
    // TODO: implement
    return -1;
}

// Exchange two complete frame values with zero copies and exactly three moves.
void swap_frames(FrameBuffer& a, FrameBuffer& b) {
    // TODO: implement
}

int main() {
    // Plain construction is neither a copy nor a move.
    FrameBuffer::reset_counters();
    FrameBuffer fb(4, 3, 2);  // 4x3 frame, every pixel = 2
    assert(fb.width() == 4 && fb.height() == 3 && fb.size() == 12);
    assert(FrameBuffer::copies_made == 0 && FrameBuffer::moves_made == 0);

    // push_back(fb) copies; push_back(std::move(fb)) steals.
    {
        std::vector<FrameBuffer> frames;
        frames.reserve(2);  // no reallocation -> counters see ONLY our push_backs
        const unsigned char* buffer_before = fb.pixels();

        FrameBuffer::reset_counters();
        frames.push_back(fb);  // lvalue argument -> copy constructor
        assert(FrameBuffer::copies_made == 1 && FrameBuffer::moves_made == 0);
        assert(fb.size() == 12);                      // original untouched
        assert(frames[0].pixels() != buffer_before);  // deep copy: its OWN buffer

        frames.push_back(std::move(fb));  // rvalue argument -> move constructor
        assert(FrameBuffer::copies_made == 1 && FrameBuffer::moves_made == 1);
        assert(frames[1].pixels() == buffer_before);  // the SAME heap buffer: stolen
        // This capstone defines an empty moved-from invariant for FrameBuffer.
        assert(fb.size() == 0 && fb.width() == 0 && fb.height() == 0);
    }

    // make_frame: returning by value is free (C++17 guaranteed elision).
    {
        FrameBuffer::reset_counters();
        FrameBuffer frame = make_frame(8, 2);
        assert(frame.width() == 8 && frame.height() == 2 && frame.size() == 16);
        assert(FrameBuffer::copies_made == 0 && FrameBuffer::moves_made == 0);
    }

    // consume: the caller chooses copy or move at the call site.
    {
        FrameBuffer frame(2, 2, 7);  // 4 pixels of 7 -> checksum 28
        FrameBuffer::reset_counters();

        assert(consume(frame) == 28);  // lvalue -> the parameter is a COPY
        assert(FrameBuffer::copies_made == 1 && FrameBuffer::moves_made == 0);
        assert(frame.size() == 4);  // we still have our frame

        assert(consume(std::move(frame)) == 28);  // rvalue -> the parameter MOVED
        assert(FrameBuffer::copies_made == 1 && FrameBuffer::moves_made == 1);
        assert(frame.size() == 0);  // gave it away for good

        FrameBuffer::reset_counters();
        assert(consume(make_frame(3, 1)) == 0);  // temporary -> built in place
        assert(FrameBuffer::copies_made == 0 && FrameBuffer::moves_made == 0);
    }

    // swap_frames: three moves, zero copies, zero pixels touched.
    {
        FrameBuffer a(2, 1, 5);  // 2 pixels of 5
        FrameBuffer b(3, 1, 9);  // 3 pixels of 9
        FrameBuffer::reset_counters();
        swap_frames(a, b);
        assert(FrameBuffer::copies_made == 0 && FrameBuffer::moves_made == 3);
        assert(a.width() == 3 && a.size() == 3 && a.bytes().front() == 9);
        assert(b.width() == 2 && b.size() == 2 && b.bytes().front() == 5);
    }

    std::cout << "ALL TESTS PASSED" << std::endl;
    return 0;
}
