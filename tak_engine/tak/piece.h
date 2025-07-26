#pragma once

// Piece representation for the game of Tak.
class Piece
{
public:
	enum Value : int8_t {
		NONE = 0,
		W_FLAT = 1, // 0b001
		W_WALL = 2, // 0b010
		W_CAP = 3,  // 0b011
		B_FLAT = -1, // 0b101
		B_WALL = -2, // 0b110
		B_CAP = -3,  // 0b111
	};

	Piece() = default;
	constexpr Piece(Value v) : p(v) {}
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

	constexpr bool get_player() const {
		if (p == Piece::NONE)
			return 0; // no player
		return (p > Piece::NONE) ? 1 : -1; // 1 for white, -1 for black
	}

	constexpr int8_t to_int() const {
		return static_cast<int8_t>(p);
	}

	constexpr uint8_t get_type_bits() const {
		return static_cast<uint8_t>(p & 0b11);
	}

private:
	Value p = NONE; // default to NONE
};


