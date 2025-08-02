#pragma once
#include <cstdint>

// bordered layout: (1 = board, 0 = border)
// This helps with flood fill!
// 00000000
// 00000000
// 00111111
// 00111111
// 00111111
// 00111111
// 00111111
// 00111111
struct bitboard_t {
    constexpr bitboard_t() : value(0) {}
	constexpr bitboard_t(uint64_t bits) : value(bits) {}
    constexpr operator uint64_t() const { return value; }

    // compound assignments
    constexpr bitboard_t& operator|=(bitboard_t other) {
        value |= other.value;
        return *this;
    }
    constexpr bitboard_t& operator&=(bitboard_t other) {
        value &= other.value;
        return *this;
    }
    constexpr bitboard_t& operator>>= (bitboard_t other) {
        value >>= other.value;
        return *this;
    }

    constexpr uint32_t count() const { return std::popcount(value); }

private:
	uint64_t value;
};
