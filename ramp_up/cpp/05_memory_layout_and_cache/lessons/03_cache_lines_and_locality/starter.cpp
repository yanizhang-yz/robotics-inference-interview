// DRILL BRIEF
// Concept: Model how stride changes useful data per cache line.
// Scenario: Compare a packed score scan with one float every 64 bytes.
// Implement: cache_lines_touched.
// Behavior: Count unique arithmetic line indices for a fixed access pattern.
// Example: the asserted stride-1 and stride-16 accesses touch their literal line counts. Edge: zero accesses touch zero lines.
// Interview focus: Reason from addresses and useful bytes, never timing ratios.
// Tests: main checks contiguous, strided, and empty access patterns.
// Run: PRACTICE=1 uv run pytest ramp_up/cpp/05_memory_layout_and_cache/lessons/03_cache_lines_and_locality -q
// Done when: The test passes and the program prints ALL TESTS PASSED.

#include <cassert>
#include <cstddef>
#include <iostream>

std::size_t cache_lines_touched(
    std::size_t elements,
    std::size_t stride_elements,
    std::size_t element_bytes,
    std::size_t line_bytes
) {
    // TODO: count the distinct line index for each arithmetic byte offset.
    (void)elements;
    (void)stride_elements;
    (void)element_bytes;
    (void)line_bytes;
    return 0;
}

int main() {
    assert(cache_lines_touched(16, 1, sizeof(float), 64) == 1);
    assert(cache_lines_touched(16, 16, sizeof(float), 64) == 16);
    assert(cache_lines_touched(0, 1, sizeof(float), 64) == 0);
    std::cout << "ALL TESTS PASSED\n";
}
