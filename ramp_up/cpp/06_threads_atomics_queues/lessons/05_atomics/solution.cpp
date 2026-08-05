#include <atomic>
#include <cassert>
#include <iostream>
#include <thread>
#include <vector>

int elect_publisher(int threads) {
    std::atomic<bool> claimed{false};
    std::atomic<int> winners{0};
    std::vector<std::thread> workers;
    for (int i = 0; i < threads; ++i) {
        workers.emplace_back([&] {
            bool expected = false;
            if (claimed.compare_exchange_strong(expected, true)) {
                ++winners;
            }
        });
    }
    for (std::thread& worker : workers) {
        worker.join();
    }
    return winners.load();
}

int main() {
    assert(elect_publisher(1) == 1);
    assert(elect_publisher(8) == 1);
    std::cout << "ALL TESTS PASSED\n";
}
