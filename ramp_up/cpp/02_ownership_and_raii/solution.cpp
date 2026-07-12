// 02_ownership_and_raii — reference solution.
//
// Compile & run standalone:
//   clang++ -std=c++17 -Wall -o /tmp/raii solution.cpp && /tmp/raii
// Prints ALL TESTS PASSED when every assert holds.

#include <cassert>
#include <cstddef>
#include <iostream>
#include <memory>
#include <string>
#include <utility>
#include <vector>

// A Buffer owns a heap-allocated int array via std::unique_ptr<int[]>.
// JAVA: a class with an int[] field; the GC frees the array whenever it likes.
// C++:  unique_ptr<int[]> frees the array in Buffer's destructor — which the
//       compiler writes FOR us because every member cleans up after itself
//       (the "rule of zero"). No delete[], no leak, deterministic.
class Buffer {
public:
    explicit Buffer(std::size_t n)
        : size_(n), data_(std::make_unique<int[]>(n)) {}  // zero-initialized, like Java's new int[n]

    std::size_t size() const { return size_; }  // const: promises not to mutate *this

    void fill(int v) {
        for (std::size_t i = 0; i < size_; ++i) {
            data_[i] = v;
        }
    }

    long long sum() const {
        long long total = 0;
        for (std::size_t i = 0; i < size_; ++i) {
            total += data_[i];
        }
        return total;
    }

private:
    std::size_t size_ = 0;
    std::unique_ptr<int[]> data_;
};

// Factory: heap-allocates a Buffer and hands OWNERSHIP to the caller.
// JAVA: static Buffer create(int n) { return new Buffer(n); } — the returned
//       reference is just another alias; nobody "owns" the object.
// C++:  the unique_ptr in the return value IS the ownership. Returning it moves
//       it out — no std::move needed on a return of a local.
std::unique_ptr<Buffer> makeBuffer(std::size_t n) {
    return std::make_unique<Buffer>(n);
}

// Takes the unique_ptr BY VALUE: calling this CONSUMES the caller's pointer.
// The caller must write moveBuffer(std::move(theirPtr)) — ownership transfer is
// visible at the call site, and their pointer is null afterwards.
// JAVA: impossible to express — passing a reference never revokes the caller's.
long long moveBuffer(std::unique_ptr<Buffer> owned) {
    long long total = owned->sum();
    return total;
    // `owned` dies here -> Buffer destroyed NOW. Deterministic, unlike finalize().
}

// RAII scope logger: constructor records "enter", destructor records "exit".
// JAVA: try { log.add("enter"); ... } finally { log.add("exit"); }
// C++:  the destructor IS the finally block — it runs at the closing brace,
//       even if an exception unwinds the scope. Any class can do this; no
//       AutoCloseable interface required.
class ScopedLogger {
public:
    explicit ScopedLogger(std::vector<std::string>& log) : log_(log) {
        log_.push_back("enter");
    }

    ~ScopedLogger() {
        log_.push_back("exit");
    }

    // Copying a "scope guard" makes no sense — forbid it at compile time.
    ScopedLogger(const ScopedLogger&) = delete;
    ScopedLogger& operator=(const ScopedLogger&) = delete;

private:
    std::vector<std::string>& log_;  // borrowed, not owned: no cleanup duty here
};

int main() {
    // Buffer: stack-constructed object owning heap memory
    {
        Buffer b(5);
        assert(b.size() == 5);
        assert(b.sum() == 0);  // make_unique<int[]> zero-initializes, like Java arrays
        b.fill(3);
        assert(b.sum() == 15);

        Buffer empty(0);
        assert(empty.size() == 0);
        assert(empty.sum() == 0);
    }  // <- b's destructor frees the heap array RIGHT HERE. No GC involved.

    // makeBuffer: ownership flows out of a factory
    {
        auto buf = makeBuffer(4);
        assert(buf != nullptr);
        assert(buf->size() == 4);
        buf->fill(2);
        assert(buf->sum() == 8);
    }  // <- buf dies, Buffer destroyed

    // moveBuffer: passing unique_ptr by value transfers ownership
    {
        auto buf = makeBuffer(3);
        buf->fill(7);
        long long total = moveBuffer(std::move(buf));  // hand it over — visibly
        assert(total == 21);
        assert(buf == nullptr);  // moved-from unique_ptr is GUARANTEED empty
    }

    // ScopedLogger: destructor as deterministic scope-exit hook
    {
        std::vector<std::string> log;
        {
            ScopedLogger logger(log);
            assert(log.size() == 1);
            assert(log[0] == "enter");
        }  // <- destructor runs at this exact brace
        assert(log.size() == 2);
        assert(log[1] == "exit");

        // Nested scopes: destructors fire in reverse construction order.
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
