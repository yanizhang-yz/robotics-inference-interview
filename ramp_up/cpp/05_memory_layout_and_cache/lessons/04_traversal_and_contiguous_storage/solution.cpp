#include <cassert>
#include <cstddef>
#include <iostream>
#include <vector>

std::vector<std::size_t> row_major_offsets(
    std::size_t rows, std::size_t cols
) {
    std::vector<std::size_t> offsets;
    for (std::size_t row = 0; row < rows; ++row) {
        for (std::size_t col = 0; col < cols; ++col) {
            offsets.push_back(row * cols + col);
        }
    }
    return offsets;
}

std::vector<std::size_t> column_major_offsets(
    std::size_t rows, std::size_t cols
) {
    std::vector<std::size_t> offsets;
    for (std::size_t col = 0; col < cols; ++col) {
        for (std::size_t row = 0; row < rows; ++row) {
            offsets.push_back(row * cols + col);
        }
    }
    return offsets;
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
