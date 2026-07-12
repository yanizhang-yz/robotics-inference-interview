// 02_ownership_and_raii — YOUR attempt. Fill in the TODO bodies, then run:
//   PRACTICE=1 uv run pytest ramp_up/cpp/02_ownership_and_raii -v
// Or compile and run directly:
//   clang++ -std=c++17 -Wall -o /tmp/raii starter.cpp && /tmp/raii
// The stubs compile as-is but fail main()'s asserts until you implement them.
// You're done when the last line printed is: ALL TESTS PASSED

#include <cassert>
#include <cstddef>
#include <iostream>
#include <memory>
#include <string>
#include <utility>
#include <vector>

// A Buffer owns a heap-allocated int array via std::unique_ptr<int[]>.
// JAVA: a class with an int[] field; the GC frees the array whenever it likes.
// C++:  store std::unique_ptr<int[]> — its destructor frees the array, so you
//       never write delete[] (the "rule of zero"). Allocate zero-initialized
//       storage with std::make_unique<int[]>(n).
class Buffer {
public:
    explicit Buffer(std::size_t n) {
        // TODO: remember n in size_ and allocate data_ with std::make_unique<int[]>(n)
        (void)n;
        (void)size_;  // delete me once size_ is really used (silences -Wunused-private-field)
    }

    std::size_t size() const {
        // TODO: return the element count
        return 0;
    }

    void fill(int v) {
        // TODO: set every element to v (index data_ like a plain array: data_[i])
        (void)v;
    }

    long long sum() const {
        // TODO: add up all elements into a long long
        return 0;
    }

private:
    std::size_t size_ = 0;
    std::unique_ptr<int[]> data_;
};

// Factory: heap-allocates a Buffer and hands OWNERSHIP to the caller.
// JAVA: static Buffer create(int n) { return new Buffer(n); } — the returned
//       reference is just another alias; nobody "owns" the object.
// C++:  return std::make_unique<Buffer>(n). Returning a local unique_ptr moves
//       it out automatically — do NOT write std::move on the return.
std::unique_ptr<Buffer> makeBuffer(std::size_t n) {
    // TODO: implement
    (void)n;
    return nullptr;
}

// Takes the unique_ptr BY VALUE: calling this CONSUMES the caller's pointer.
// The caller must write moveBuffer(std::move(theirPtr)) — ownership transfer is
// visible at the call site, and their pointer is null afterwards.
// JAVA: impossible to express — passing a reference never revokes the caller's.
// C++:  return owned->sum(); the Buffer is destroyed when `owned` dies at the
//       end of this function — deterministic, unlike finalize().
long long moveBuffer(std::unique_ptr<Buffer> owned) {
    // TODO: implement
    (void)owned;
    return 0;
}

// RAII scope logger: constructor records "enter", destructor records "exit".
// JAVA: try { log.add("enter"); ... } finally { log.add("exit"); }
// C++:  the destructor IS the finally block — it runs at the closing brace,
//       even if an exception unwinds the scope. push_back onto log_ in the
//       constructor body and destructor body.
class ScopedLogger {
public:
    explicit ScopedLogger(std::vector<std::string>& log) : log_(log) {
        // TODO: record "enter"
        (void)log_;  // delete me once log_ is really used (silences -Wunused-private-field)
    }

    ~ScopedLogger() {
        // TODO: record "exit"
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
