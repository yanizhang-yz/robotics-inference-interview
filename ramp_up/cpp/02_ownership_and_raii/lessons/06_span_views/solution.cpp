#include <cassert>
#include <cmath>
#include <iostream>
#include <span>
#include <vector>

void scale_in_place(std::span<float> values, float gain) {
    for (float& value : values) value *= gain;
}

double mean(std::span<const float> values) {
    if (values.empty()) return 0.0;
    double total = 0.0;
    for (float value : values) total += value;
    return total / static_cast<double>(values.size());
}

int main() {
    std::vector<float> tensor{1.0F, 2.0F, 3.0F, 4.0F, 5.0F};
    scale_in_place(std::span<float>(tensor).subspan(1, 3), 10.0F);
    assert((tensor == std::vector<float>{1.0F, 20.0F, 30.0F, 40.0F, 5.0F}));
    assert(std::abs(mean(tensor) - 19.2) < 1e-9);
    assert(mean(std::span<const float>{}) == 0.0);
    std::cout << "ALL TESTS PASSED\n";
}
