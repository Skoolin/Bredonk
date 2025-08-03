#pragma once
#include <cstdint>
#include "piece.h"

// 
struct stack_t {
    constexpr stack_t() : value(0) {}
    constexpr stack_t(uint64_t bits) : value(bits) {}
    constexpr operator uint64_t() const { return value; }

    constexpr void set(int i, const Piece p) {
        uint64_t bit = 1ULL << i;
        value = (p == Piece::B_FLAT) ? (value | bit) : (value & ~bit);
    }

    constexpr const Piece operator[](int i) const {
        return (value & 1ULL << i) ? Piece::B_FLAT : Piece::W_FLAT;
    }

    // compound assignments
    constexpr stack_t& operator|=(stack_t other) {
        value |= other.value;
        return *this;
    }
    constexpr stack_t& operator&=(stack_t other) {
        value &= other.value;
        return *this;
    }
    constexpr stack_t& operator>>= (stack_t other) {
        value >>= other.value;
        return *this;
    }
    constexpr stack_t& operator <<= (stack_t other) {
        value >>= other.value;
        return *this;
    }
private:
    uint64_t value;
};
