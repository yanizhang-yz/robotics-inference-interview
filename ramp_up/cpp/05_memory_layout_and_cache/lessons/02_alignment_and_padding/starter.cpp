// DRILL BRIEF
// Concept: Account for alignment padding and reduce it by field reordering.
// Scenario: Pack many fixed-shape inference metadata records more densely.
// Implement: Reorder GoodOrder and implement padded_sizes.
// Behavior: Preserve the same fields while making GoodOrder smaller.
// Interview focus: Calculate aligned offsets; the compiler never reorders fields.
// Tests: main compares field values, alignments, reported sizes, and density.
// Run: PRACTICE=1 uv run pytest ramp_up/cpp/05_memory_layout_and_cache/lessons/02_alignment_and_padding -q
// Done when: The test passes and the program prints ALL TESTS PASSED.

#include <cassert>
#include <cstddef>
#include <iostream>
#include <utility>

struct BadOrder {
    char valid;
    double timestamp;
    char source;
    int id;
};

// TODO: reorder these same four fields from largest alignment to smallest.
struct GoodOrder {
    char valid;
    double timestamp;
    char source;
    int id;
};

std::pair<std::size_t, std::size_t> padded_sizes() {
    // TODO: report the compiler-selected sizes of both record layouts.
    return {0, 0};
}

int main() {
    const BadOrder bad{'y', 12.5, 'c', 7};
    // Keep these values while updating the initializer to match your order.
    const GoodOrder good{'y', 12.5, 'c', 7};
    assert(bad.valid == good.valid && bad.timestamp == good.timestamp);
    assert(bad.source == good.source && bad.id == good.id);
    assert(alignof(BadOrder) >= alignof(double));
    assert(alignof(GoodOrder) >= alignof(double));

    const auto [bad_size, good_size] = padded_sizes();
    assert(bad_size == sizeof(BadOrder));
    assert(good_size == sizeof(GoodOrder));
    assert(good_size < bad_size);
    std::cout << "ALL TESTS PASSED\n";
}
