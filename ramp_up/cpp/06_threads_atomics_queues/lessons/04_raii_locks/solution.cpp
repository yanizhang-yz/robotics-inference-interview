#include <cassert>
#include <iostream>
#include <mutex>
#include <stdexcept>

bool lock_released_after_exception(std::timed_mutex& mutex) {
    try {
        std::lock_guard<std::timed_mutex> lock(mutex);
        throw std::runtime_error("read failed");
    } catch (const std::runtime_error&) {
    }

    const bool acquired = mutex.try_lock();
    if (acquired) {
        mutex.unlock();
    }
    return acquired;
}

int main() {
    std::timed_mutex mutex;
    assert(lock_released_after_exception(mutex));
    std::cout << "ALL TESTS PASSED\n";
}
