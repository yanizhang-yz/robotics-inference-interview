// 03_move_semantics_rule_of_five — YOUR attempt. Fill in the TODO bodies, then run:
//   PRACTICE=1 uv run pytest ramp_up/cpp/03_move_semantics_rule_of_five -v
// Or compile and run directly:
//   clang++ -std=c++17 -Wall -o /tmp/moves starter.cpp && /tmp/moves
// The stubs compile as-is but fail main()'s asserts until you implement them.
// You're done when the last line printed is: ALL TESTS PASSED

#include <cassert>
#include <cstddef>
#include <iostream>
#include <utility>
#include <vector>

// FrameBuffer owns one grayscale image: width*height bytes in a std::vector.
// Normally a vector member means Rule of Zero (write NONE of the five) — you
// hand-write all five here because they're instrumented with counters, and
// because writing them once, correctly, is the drill.
class FrameBuffer {
public:
    // Proof instruments: bump copies_made in BOTH copy operations and
    // moves_made in BOTH move operations, so main() can prove which ran.
    inline static int copies_made = 0;  // inline static (C++17): defined right here
    inline static int moves_made = 0;
    static void reset_counters() { copies_made = 0; moves_made = 0; }

    // Given: the ordinary constructor is not one of the five.
    FrameBuffer(int width, int height, unsigned char fill = 0)
        : width_(width),
          height_(height),
          data_(static_cast<std::size_t>(width) * height, fill) {}

    // ---- The Rule of Five: your work ----------------------------------

    // 1) Destructor — already correct: data_ is a vector, it frees its own
    //    heap buffer. Your work is the other four.
    ~FrameBuffer() = default;

    // 2) Copy constructor: DEEP copy — copy width_/height_/data_ from other
    //    (member initializer list preferred; copying the vector duplicates
    //    every pixel), then ++copies_made.
    FrameBuffer(const FrameBuffer& other) {
        // TODO: implement
    }

    // 3) Copy assignment: same deep copy into an already-built object.
    //    Guard against self-assignment (`if (this != &other)`), count the call.
    FrameBuffer& operator=(const FrameBuffer& other) {
        // TODO: implement
        return *this;
    }

    // 4) Move constructor: STEAL other's guts — std::move(other.data_) hands
    //    the heap buffer over without touching pixels — then leave other as a
    //    valid 0x0 empty frame, and ++moves_made. Keep the noexcept: vector
    //    only moves elements during reallocation if the move ctor cannot throw.
    FrameBuffer(FrameBuffer&& other) noexcept {
        // TODO: implement
    }

    // 5) Move assignment: same steal into an already-built object.
    //    Self-assignment guard, zero out other, count the call.
    FrameBuffer& operator=(FrameBuffer&& other) noexcept {
        // TODO: implement
        return *this;
    }

    int width() const { return width_; }
    int height() const { return height_; }
    std::size_t size() const { return data_.size(); }  // pixel count
    const unsigned char* pixels() const { return data_.data(); }  // buffer address
    const std::vector<unsigned char>& bytes() const { return data_; }

private:
    int width_ = 0;
    int height_ = 0;
    std::vector<unsigned char> data_;
};

// make_frame(8, 2) -> an 8x2 zero-filled frame, returned BY VALUE.
// JAVA: returning an object hands back a reference — cheap by definition.
// C++:  return the freshly constructed FrameBuffer directly; C++17 guarantees
//       copy elision (built in the caller's variable, zero copies, zero
//       moves). Do NOT write `return std::move(...)` — that disables it.
FrameBuffer make_frame(int width, int height) {
    // TODO: implement
    return FrameBuffer(0, 0);
}

// consume(fb) -> sum of every pixel byte. Takes its argument BY VALUE on
// purpose (the "sink" idiom): the caller picks the cost at the call site —
// consume(frame) copies once, consume(std::move(frame)) moves once.
// Loop over fb.bytes(), accumulate into a long long.
long long consume(FrameBuffer fb) {
    // TODO: implement
    return -1;
}

// Swap two frames without copying a single pixel: exactly three moves via a
// temporary — tmp steals a, a steals b, b steals tmp (how std::swap works).
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
        // Moved-from: valid but empty — safe to destroy or assign to, nothing more.
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
