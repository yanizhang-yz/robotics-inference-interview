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
        : width_(width),
          height_(height),
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
    return Frame(width, height);
}

int main() {
    Frame::reset_counters();
    Frame frame = make_frame(5, 4);
    assert(frame.width() == 5 && frame.height() == 4 && frame.size() == 20);
    assert(Frame::copies == 0 && Frame::moves == 0);

    std::cout << "ALL TESTS PASSED\n";
}
