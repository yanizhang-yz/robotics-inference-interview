// 03_move_semantics_rule_of_five — reference solution.
//
// Compile & run standalone:
//   clang++ -std=c++17 -Wall -o /tmp/moves solution.cpp && /tmp/moves
// Prints ALL TESTS PASSED when every assert holds.

#include <cassert>
#include <cstddef>
#include <iostream>
#include <utility>
#include <vector>

// FrameBuffer owns one grayscale image: width*height bytes in a std::vector.
// A vector member would normally make this a Rule of Zero class (write NONE of
// the five special members) — we hand-write all five here only because the
// drill instruments them with counters. Observable behavior in copy/move
// (counting, logging) is the one legitimate reason left to write them.
class FrameBuffer {
public:
    // Proof instruments: every copy/move bumps a counter so main() can PROVE
    // which special member ran. inline static (C++17): define-and-initialize
    // right here in the class, no out-of-class definition line needed.
    inline static int copies_made = 0;
    inline static int moves_made = 0;
    static void reset_counters() { copies_made = 0; moves_made = 0; }

    FrameBuffer(int width, int height, unsigned char fill = 0)
        : width_(width),
          height_(height),
          data_(static_cast<std::size_t>(width) * height, fill) {}

    // ---- The Rule of Five ---------------------------------------------

    // 1) Destructor. data_ is a vector, so it frees its own heap buffer —
    //    nothing left for us to do. Defaulted so all five stay visible.
    ~FrameBuffer() = default;

    // 2) Copy constructor: a deep copy — copying data_ allocates a fresh
    //    buffer and duplicates every pixel.
    FrameBuffer(const FrameBuffer& other)
        : width_(other.width_), height_(other.height_), data_(other.data_) {
        ++copies_made;
    }

    // 3) Copy assignment: same deep copy, into an already-built object.
    FrameBuffer& operator=(const FrameBuffer& other) {
        ++copies_made;         // counts calls, like the copy constructor
        if (this != &other) {  // self-assignment guard: `fb = fb` must be safe
            width_ = other.width_;
            height_ = other.height_;
            data_ = other.data_;
        }
        return *this;
    }

    // 4) Move constructor: STEAL other's buffer — vector's move is a pointer
    //    handoff, no pixels touched — then leave other valid-but-empty.
    //    noexcept matters: vector only moves elements during reallocation if
    //    the move constructor promises not to throw (README §5).
    FrameBuffer(FrameBuffer&& other) noexcept
        : width_(other.width_),
          height_(other.height_),
          data_(std::move(other.data_)) {  // vector move ctor: source guaranteed empty
        other.width_ = 0;   // WE define the moved-from state:
        other.height_ = 0;  // a valid 0x0 frame with no pixels
        ++moves_made;
    }

    // 5) Move assignment: same steal, into an already-built object.
    FrameBuffer& operator=(FrameBuffer&& other) noexcept {
        ++moves_made;
        if (this != &other) {
            width_ = other.width_;
            height_ = other.height_;
            data_ = std::move(other.data_);
            other.data_.clear();  // move ASSIGNMENT leaves the source vector
                                  // unspecified (in practice empty); clear()
                                  // upgrades that to a guarantee
            other.width_ = 0;
            other.height_ = 0;
        }
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
// C++:  returning by value looks like it should copy the whole frame, but
//       C++17 GUARANTEES copy elision for a freshly constructed return value:
//       the frame is built directly in the caller's variable. Zero copies,
//       zero moves — main() proves it. Never "help" with `return std::move(...)`:
//       that DISABLES the optimization (clang -Wall: "moving a local object in
//       a return statement prevents copy elision").
FrameBuffer make_frame(int width, int height) {
    return FrameBuffer(width, height);
}

// consume(fb) -> sum of every pixel byte. Takes its argument BY VALUE: the
// "sink" idiom for parameters a function keeps or uses up. The CALLER picks
// the cost at the call site:
//   consume(frame)             -> parameter copy-constructed   (1 copy)
//   consume(std::move(frame))  -> parameter move-constructed   (1 move)
//   consume(make_frame(3, 1))  -> parameter built in place     (0 and 0)
long long consume(FrameBuffer fb) {
    long long checksum = 0;
    for (unsigned char byte : fb.bytes()) {
        checksum += byte;
    }
    return checksum;
}  // fb's destructor runs here — the function really does consume the frame

// Swap two frames without touching a single pixel: exactly three moves —
// which is precisely how std::swap is implemented. A copy-based swap of two
// 2 MB frames would shovel 6 MB of pixels; this shuffles three sets of
// {pointer, size, capacity}.
void swap_frames(FrameBuffer& a, FrameBuffer& b) {
    FrameBuffer tmp = std::move(a);  // move #1: tmp steals a's buffer
    a = std::move(b);                // move #2: a steals b's buffer
    b = std::move(tmp);              // move #3: b steals tmp's buffer
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
