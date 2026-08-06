#include <algorithm>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <iostream>
#include <utility>

class RawFrame {
public:
    inline static int live_allocations = 0;

    RawFrame(std::size_t size, std::uint8_t fill)
        : size_(size), data_(size ? new std::uint8_t[size] : nullptr) {
        if (data_ != nullptr) {
            std::fill(data_, data_ + size_, fill);
            ++live_allocations;
        }
    }
    ~RawFrame() {
        if (data_ != nullptr) --live_allocations;
        delete[] data_;
    }
    RawFrame(const RawFrame& other)
        : size_(other.size_),
          data_(other.size_ ? new std::uint8_t[other.size_] : nullptr) {
        if (data_ != nullptr) {
            std::copy(other.data_, other.data_ + other.size_, data_);
            ++live_allocations;
        }
    }
    RawFrame& operator=(const RawFrame& other) {
        if (this == &other) return *this;
        RawFrame copy(other);
        swap(copy);
        return *this;
    }
    RawFrame(RawFrame&& other) noexcept
        : size_(std::exchange(other.size_, 0)),
          data_(std::exchange(other.data_, nullptr)) {}
    RawFrame& operator=(RawFrame&& other) noexcept {
        if (this != &other) {
            if (data_ != nullptr) --live_allocations;
            delete[] data_;
            size_ = std::exchange(other.size_, 0);
            data_ = std::exchange(other.data_, nullptr);
        }
        return *this;
    }

    void swap(RawFrame& other) noexcept {
        using std::swap;
        swap(size_, other.size_);
        swap(data_, other.data_);
    }

    std::size_t size() const { return size_; }
    std::uint8_t* data() { return data_; }
    const std::uint8_t* data() const { return data_; }

private:
    std::size_t size_ = 0;
    std::uint8_t* data_ = nullptr;
};

int main() {
    assert(RawFrame::live_allocations == 0);
    {
        RawFrame original(4, 7);
        assert(RawFrame::live_allocations == 1);

        RawFrame copied(original);
        assert(copied.size() == 4);
        assert(copied.data() != nullptr);
        assert(copied.data() != original.data());
        assert(copied.data()[0] == 7);
        copied.data()[0] = 8;
        assert(original.data()[0] == 7);

        RawFrame assigned(2, 1);
        assigned = original;
        assert(assigned.size() == 4 && assigned.data() != nullptr);
        assert(assigned.data() != original.data());
        assigned = assigned;
        assert(assigned.size() == 4 && assigned.data()[0] == 7);

        const std::uint8_t* original_address = original.data();
        RawFrame moved(std::move(original));
        assert(moved.data() == original_address);
        assert(original.size() == 0 && original.data() == nullptr);

        const std::uint8_t* copied_address = copied.data();
        assigned = std::move(copied);
        assert(assigned.data() == copied_address);
        assert(copied.size() == 0 && copied.data() == nullptr);
        assigned = std::move(assigned);
        assert(assigned.data() == copied_address);
    }
    assert(RawFrame::live_allocations == 0);

    std::cout << "ALL TESTS PASSED\n";
}
