// DRILL BRIEF
// Concept: Trace access order through row-major contiguous storage.
// Scenario: Choose loop nesting for a flat image or tensor plane.
// Implement: row_major_offsets and column_major_offsets.
// Behavior: Visit every row-major offset once in the requested loop order.
// Interview focus: Explain locality changes without changing the loop's Big-O.
// Tests: main checks literal access orders for a 2-by-3 buffer.
// Run: PRACTICE=1 uv run pytest ramp_up/cpp/05_memory_layout_and_cache/lessons/04_traversal_and_contiguous_storage -q
// Done when: The test passes and the program prints ALL TESTS PASSED.

#include <cassert>
#include <cstddef>
#include <iostream>
#include <vector>

std::vector<std::size_t> row_major_offsets(
    std::size_t rows, std::size_t cols
) {
    // TODO: visit rows outside and columns inside.
    (void)rows;
    (void)cols;
    return {};
}

std::vector<std::size_t> column_major_offsets(
    std::size_t rows, std::size_t cols
) {
    // TODO: visit columns outside and rows inside, keeping row-major indexing.
    (void)rows;
    (void)cols;
    return {};
}

int main() {
    const std::vector<std::size_t> row_order{0, 1, 2, 3, 4, 5};
    const std::vector<std::size_t> column_order{0, 3, 1, 4, 2, 5};
    assert(row_major_offsets(2, 3) == row_order);
    assert(column_major_offsets(2, 3) == column_order);
    assert(row_major_offsets(2, 3).size() == 6);
    assert(column_major_offsets(2, 3).size() == 6);
    std::cout << "ALL TESTS PASSED\n";
}
