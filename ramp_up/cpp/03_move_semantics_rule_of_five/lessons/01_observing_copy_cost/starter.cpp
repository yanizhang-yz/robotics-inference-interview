// DRILL BRIEF
// Concept: A by-value owning parameter copies its payload; const& only borrows it.
// Scenario: Compare two read-only inspection boundaries for a camera frame.
// Implement: inspect_by_value and inspect_by_const_ref using the supplied checksum.
// Behavior: Equal checksums, with exactly one copy by value and zero by const reference.
// Interview focus: Choose a signature for a large frame and identify the copy boundary.
// Tests: main resets counters immediately before each measured call and checks the cost.
// Run: PRACTICE=1 uv run pytest ramp_up/cpp/03_move_semantics_rule_of_five/lessons/01_observing_copy_cost -q
// Done when: The test passes and the program prints ALL TESTS PASSED.

#include <cassert>
#include <cstdint>
#include <iostream>
#include <vector>

class TrackedFrame {
public:
    inline static int copies = 0;

    TrackedFrame(int width, int height, std::uint8_t fill)
        : pixels_(static_cast<std::size_t>(width) * height, fill) {}
    TrackedFrame(const TrackedFrame& other) : pixels_(other.pixels_) { ++copies; }

    long long checksum() const {
        long long total = 0;
        for (std::uint8_t pixel : pixels_) total += pixel;
        return total;
    }
    static void reset_counters() { copies = 0; }

private:
    std::vector<std::uint8_t> pixels_;
};

long long inspect_by_value(TrackedFrame frame) {
    // TODO: inspect the parameter.
    (void)frame;
    return 0;
}

long long inspect_by_const_ref(const TrackedFrame& frame) {
    // TODO: inspect the borrowed frame.
    (void)frame;
    return 0;
}

int main() {
    TrackedFrame frame(3, 2, 5);
    TrackedFrame::reset_counters();
    const long long by_value = inspect_by_value(frame);
    assert(by_value == 30 && TrackedFrame::copies == 1);

    TrackedFrame::reset_counters();
    const long long by_ref = inspect_by_const_ref(frame);
    assert(by_ref == by_value && TrackedFrame::copies == 0);
    std::cout << "ALL TESTS PASSED\n";
}
