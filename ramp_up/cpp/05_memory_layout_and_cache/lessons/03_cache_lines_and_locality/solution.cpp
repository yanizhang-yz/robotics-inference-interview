#include <cassert>
#include <cstddef>
#include <iostream>
#include <set>

std::size_t cache_lines_touched(
    std::size_t elements,
    std::size_t stride_elements,
    std::size_t element_bytes,
    std::size_t line_bytes
) {
    std::set<std::size_t> lines;
    for (std::size_t i = 0; i < elements; ++i) {
        lines.insert((i * stride_elements * element_bytes) / line_bytes);
    }
    return lines.size();
}

int main() {
    assert(cache_lines_touched(16, 1, sizeof(float), 64) == 1);
    assert(cache_lines_touched(16, 16, sizeof(float), 64) == 16);
    assert(cache_lines_touched(0, 1, sizeof(float), 64) == 0);
    std::cout << "ALL TESTS PASSED\n";
}
