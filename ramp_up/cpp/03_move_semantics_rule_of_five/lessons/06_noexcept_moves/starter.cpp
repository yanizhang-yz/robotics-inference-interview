// DRILL BRIEF
// Concept: vector growth prefers a non-throwing move to preserve strong guarantees.
// Scenario: Reallocate a packet queue without duplicating existing payload bytes.
// Implement: Packet's noexcept move constructor and packet_move_is_noexcept().
// Behavior: The trait is true; growth records one move and zero copies.
// Interview focus: Diagnose why a movable type unexpectedly copies on reallocation.
// Tests: main resets counters immediately before growth and verifies sizes and counts.
// Run: PRACTICE=1 uv run pytest ramp_up/cpp/03_move_semantics_rule_of_five/lessons/06_noexcept_moves -q
// Done when: The test passes and the program prints ALL TESTS PASSED.

#include <cassert>
#include <cstddef>
#include <cstdint>
#include <iostream>
#include <utility>
#include <vector>

class Packet {
public:
    inline static int copies = 0;
    inline static int moves = 0;
    Packet(std::size_t size, std::uint8_t fill) : bytes_(size, fill) {}
    Packet(const Packet& other) : bytes_(other.bytes_) { ++copies; }
    Packet(Packet&& other) noexcept {
        // TODO: transfer bytes_ and record the move.
        (void)other;
    }
    static void reset_counters() { copies = 0; moves = 0; }
    std::size_t size() const { return bytes_.size(); }

private:
    std::vector<std::uint8_t> bytes_;
};

bool packet_move_is_noexcept() {
    // TODO: query construction from Packet&& with noexcept(...).
    return false;
}

int main() {
    assert(packet_move_is_noexcept());
    std::vector<Packet> packets;
    packets.reserve(1);
    packets.emplace_back(8, 3);
    Packet::reset_counters();
    packets.emplace_back(4, 7);
    assert(Packet::moves == 1 && Packet::copies == 0);
    assert(packets[0].size() == 8 && packets[1].size() == 4);
    std::cout << "ALL TESTS PASSED\n";
}
