#include <cassert>
#include <cstddef>
#include <iostream>
#include <thread>
#include <vector>

std::vector<int> offset_samples(const std::vector<int>& samples, int offset) {
    std::vector<int> output(samples.size());
    std::vector<std::thread> workers;
    for (std::size_t i = 0; i < samples.size(); ++i) {
        workers.emplace_back(
            [&, i, offset] { output[i] = samples[i] + offset; });
    }
    for (std::thread& worker : workers) {
        worker.join();
    }
    return output;
}

int main() {
    const std::vector<int> samples{4, -1, 9, 0};
    const std::vector<int> expected{10, 5, 15, 6};

    assert(offset_samples(samples, 6) == expected);
    assert(offset_samples({}, 3).empty());
    std::cout << "ALL TESTS PASSED\n";
}
