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

struct GoodOrder {
    double timestamp;
    int id;
    char valid;
    char source;
};

std::pair<std::size_t, std::size_t> padded_sizes() {
    return {sizeof(BadOrder), sizeof(GoodOrder)};
}

int main() {
    const BadOrder bad{'y', 12.5, 'c', 7};
    const GoodOrder good{12.5, 7, 'y', 'c'};
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
