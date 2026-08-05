#include <cassert>
#include <cstddef>
#include <iostream>
#include <span>
#include <thread>
#include <vector>

long long parallel_sum(std::span<const int> values) {
    const std::size_t middle = values.size() / 2;
    long long left = 0;
    long long right = 0;

    std::thread first([&] {
        for (int value : values.first(middle)) {
            left += value;
        }
    });
    std::thread second([&] {
        for (int value : values.subspan(middle)) {
            right += value;
        }
    });

    first.join();
    second.join();
    return left + right;
}

int main() {
    const std::vector<int> empty;
    const std::vector<int> positive{1, 2, 3, 4};
    const std::vector<int> mixed{-8, 3, -2, 10};

    assert(parallel_sum(empty) == 0);
    assert(parallel_sum(positive) == 10);
    assert(parallel_sum(mixed) == 3);
    std::cout << "ALL TESTS PASSED\n";
}
