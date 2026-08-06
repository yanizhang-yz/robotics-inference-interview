#include <cassert>
#include <cstddef>
#include <iostream>
#include <vector>

std::size_t count_capacity_changes(std::size_t n, bool reserve_first) {
    std::vector<int> values;
    std::size_t changes = 0;
    std::size_t previous = values.capacity();
    if (reserve_first && n > 0) {
        values.reserve(n);
        if (values.capacity() != previous) ++changes;
        previous = values.capacity();
    }
    for (std::size_t i = 0; i < n; ++i) {
        values.push_back(static_cast<int>(i));
        if (values.capacity() != previous) {
            ++changes;
            previous = values.capacity();
        }
    }
    return changes;
}

std::size_t reallocations_with_reserve(std::size_t n) {
    return count_capacity_changes(n, true);
}

std::size_t reallocations_without_reserve(std::size_t n) {
    return count_capacity_changes(n, false);
}

int main() {
    assert(reallocations_with_reserve(0) == 0);
    assert(reallocations_without_reserve(0) == 0);
    assert(reallocations_with_reserve(100) == 1);
    const std::size_t unreserved_changes = reallocations_without_reserve(100);
    std::cout << "unreserved capacity changes: " << unreserved_changes << '\n';
    std::cout << "ALL TESTS PASSED\n";
}
