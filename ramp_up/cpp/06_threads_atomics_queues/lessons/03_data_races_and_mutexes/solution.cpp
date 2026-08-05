#include <cassert>
#include <iostream>
#include <mutex>
#include <thread>
#include <vector>

int safe_count_mutex(int threads, int iterations) {
    int counter = 0;
    std::mutex mutex;
    std::vector<std::thread> workers;
    for (int i = 0; i < threads; ++i) {
        workers.emplace_back([&] {
            for (int j = 0; j < iterations; ++j) {
                std::lock_guard<std::mutex> lock(mutex);
                ++counter;
            }
        });
    }
    for (std::thread& worker : workers) {
        worker.join();
    }
    return counter;
}

int main() {
    assert(safe_count_mutex(4, 25000) == 100000);
    assert(safe_count_mutex(1, 1000) == 1000);
    std::cout << "ALL TESTS PASSED\n";
}
