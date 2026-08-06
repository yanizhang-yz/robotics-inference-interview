// DRILL BRIEF
// Concept: A raw owning pointer requires coherent destructor, copy, and move operations.
// Scenario: Wrap a low-level frame allocation while preserving one-owner invariants.
// Implement: RawFrame's destructor, two copy operations, and two move operations.
// Behavior: Copies are deep, moves preserve addresses and empty sources, and nothing leaks.
// Example: copying four bytes produces a distinct address and moving preserves it. Edge: self-assignment, self-move, and empty ownership remain valid.
// Interview focus: Derive the Rule of Five and explain why Rule of Zero is preferable.
// Tests: main checks addresses, bytes, self-assignment, moved-from states, and allocations.
// Run: PRACTICE=1 uv run pytest ramp_up/cpp/03_move_semantics_rule_of_five/lessons/08_rule_of_five -q
// Done when: The test passes and the program prints ALL TESTS PASSED.

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
        // TODO: release an owned allocation and update the counter.
    }
    RawFrame(const RawFrame& other) {
        // TODO: deep-copy other and update the counter if allocation occurs.
        (void)other;
    }
    RawFrame& operator=(const RawFrame& other) {
        // TODO: make self-assignment safe and provide an independent copy.
        (void)other;
        return *this;
    }
    RawFrame(RawFrame&& other) noexcept {
        // TODO: take other's ownership and empty it.
        (void)other;
    }
    RawFrame& operator=(RawFrame&& other) noexcept {
        // TODO: release current ownership, then take and empty a distinct source.
        (void)other;
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
        assert(copied.size() == 4 && copied.data() != nullptr);
        assert(copied.data() != original.data() && copied.data()[0] == 7);
        copied.data()[0] = 8;
        assert(original.data()[0] == 7);

        RawFrame assigned(2, 1);
        assigned = original;
        assert(assigned.size() == 4 && assigned.data() != original.data());
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
