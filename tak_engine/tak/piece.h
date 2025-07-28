#pragma once

#include "enums.h"

// Piece representation for the game of Tak.
class Piece
{
public:
	enum Value : uint8_t {
		NONE = 0,
		W_FLAT = 1, // 0b001
		W_WALL = 2, // 0b010
		W_CAP = 3, // 0b011
		B_FLAT = 5, // 0b101
		B_WALL = 6, // 0b110
		B_CAP = 7, // 0b111
	};

	Piece() = default;

	const static uint8_t B_START = 4; // B_FLAT starts at 5, so B_START is 4

	constexpr Piece(Value value) : p(value) {}

	constexpr operator Value() const { return p; }
	explicit operator bool() const { return p != NONE; }

	constexpr bool is_flat() const {
		return p == Piece::W_FLAT || p == Piece::B_FLAT;
	}

	constexpr bool is_wall() const {
		return p == Piece::W_WALL || p == Piece::B_WALL;
	}

	constexpr bool is_capstone() const {
		return p == Piece::W_CAP || p == Piece::B_CAP;
	}

	constexpr int32_t get_player() const {
		if (p == Piece::NONE)
			return 0; // no player
		return (p >= Piece::B_FLAT) ? PLAYER_BLACK : PLAYER_WHITE;
	}

	constexpr uint8_t to_int() const {
		return static_cast<uint8_t>(p & 0b111);
	}

	constexpr uint8_t get_type_bits() const {
		return static_cast<uint8_t>(p & 0b11);
	}

private:
	Value p = NONE; // default to NONE
};


