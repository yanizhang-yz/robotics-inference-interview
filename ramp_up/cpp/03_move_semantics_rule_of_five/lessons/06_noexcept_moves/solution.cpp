#include <cassert>
#include <cstddef>
#include <cstdint>
#include <iostream>
#include <type_traits>
#include <utility>
#include <vector>

class Packet {
public:
    inline static int copies = 0;
    inline static int moves = 0;

    Packet(std::size_t size, std::uint8_t fill) : bytes_(size, fill) {}
    Packet(const Packet& other) : bytes_(other.bytes_) { ++copies; }
    Packet(Packet&& other) noexcept : bytes_(std::move(other.bytes_)) {
        ++moves;
    }

    static void reset_counters() { copies = 0; moves = 0; }
    std::size_t size() const { return bytes_.size(); }

private:
    std::vector<std::uint8_t> bytes_;
};

bool packet_move_is_noexcept() {
    return noexcept(Packet(std::declval<Packet&&>()));
}

int main() {
    assert(packet_move_is_noexcept());

    std::vector<Packet> packets;
    packets.reserve(1);
    packets.emplace_back(8, 3);
    Packet::reset_counters();
    packets.emplace_back(4, 7);
    assert(Packet::moves == 1);
    assert(Packet::copies == 0);
    assert(packets[0].size() == 8 && packets[1].size() == 4);

    std::cout << "ALL TESTS PASSED\n";
}
