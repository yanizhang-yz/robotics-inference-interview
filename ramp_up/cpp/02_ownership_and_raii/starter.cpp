// DRILL BRIEF
// Concept: Combine exclusive ownership, ownership transfer, and RAII cleanup.
// Scenario: Manage an owned integer buffer and trace nested scoped logging.
// Implement: Buffer, makeBuffer, moveBuffer, and ScopedLogger TODO bodies.
// Behavior: Buffers size/fill/sum correctly; moves consume owners; scopes log enter/exit.
// Example: Buffer(5) filled with 3 sums to 15. Edge: Buffer(0) sums to 0 and nested scopes always balance cleanup.
// Interview focus: Explain who owns each allocation and when each cleanup runs.
// Tests: main asserts buffer state, factory ownership, moved-from nullness, and log order.
// Run: PRACTICE=1 uv run pytest ramp_up/cpp/02_ownership_and_raii/test_solution.py -q
// Done when: The test run passes and the program prints ALL TESTS PASSED.

#include <cassert>
#include <cstddef>
#include <iostream>
#include <memory>
#include <string>
#include <utility>
#include <vector>

// Buffer must exclusively own exactly size() zero-initialized integers.
class Buffer {
public:
    explicit Buffer(std::size_t n) {
        // TODO: establish the size and ownership invariant.
        (void)n;
        (void)size_;
    }

    std::size_t size() const {
        // TODO: report the element count.
        return 0;
    }

    void fill(int v) {
        // TODO: set every owned element to v.
        (void)v;
    }

    long long sum() const {
        // TODO: return the sum without changing the buffer.
        return 0;
    }

private:
    std::size_t size_ = 0;
    std::unique_ptr<int[]> data_;
};

// Return one non-null exclusive owner of a Buffer containing n integers.
std::unique_ptr<Buffer> makeBuffer(std::size_t n) {
    // TODO: implement
    (void)n;
    return nullptr;
}

// Consume the exclusive owner and return the buffer's sum. Null is not an input.
long long moveBuffer(std::unique_ptr<Buffer> owned) {
    // TODO: implement
    (void)owned;
    return 0;
}

// A live ScopedLogger contributes one "enter" and exactly one matching "exit".
class ScopedLogger {
public:
    explicit ScopedLogger(std::vector<std::string>& log) : log_(log) {
        // TODO: record "enter"
        (void)log_;
    }

    ~ScopedLogger() {
        // TODO: record "exit"
    }

    // A scope guard has one lifetime and is not copyable.
    ScopedLogger(const ScopedLogger&) = delete;
    ScopedLogger& operator=(const ScopedLogger&) = delete;

private:
    std::vector<std::string>& log_;
};

int main() {
    // Buffer invariants and edge cases.
    {
        Buffer b(5);
        assert(b.size() == 5);
        assert(b.sum() == 0);
        b.fill(3);
        assert(b.sum() == 15);

        Buffer empty(0);
        assert(empty.size() == 0);
        assert(empty.sum() == 0);
    }

    // Factory ownership.
    {
        auto buf = makeBuffer(4);
        assert(buf != nullptr);
        assert(buf->size() == 4);
        buf->fill(2);
        assert(buf->sum() == 8);
    }

    // Consuming ownership.
    {
        auto buf = makeBuffer(3);
        buf->fill(7);
        long long total = moveBuffer(std::move(buf));
        assert(total == 21);
        assert(buf == nullptr);
    }

    // Scoped cleanup and nesting.
    {
        std::vector<std::string> log;
        {
            ScopedLogger logger(log);
            assert(log.size() == 1);
            assert(log[0] == "enter");
        }
        assert(log.size() == 2);
        assert(log[1] == "exit");

        // Nested scopes must balance their entries and exits.
        std::vector<std::string> nested;
        {
            ScopedLogger outer(nested);
            {
                ScopedLogger inner(nested);
            }
            assert((nested == std::vector<std::string>{"enter", "enter", "exit"}));
        }
        assert((nested == std::vector<std::string>{"enter", "enter", "exit", "exit"}));
    }

    std::cout << "ALL TESTS PASSED" << std::endl;
    return 0;
}
