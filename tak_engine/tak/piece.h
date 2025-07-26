#pragma once

enum Piece : int8_t {
	NONE = 0,
	W_FLAT = 1, // 0b001
	W_WALL = 2, // 0b010
	W_CAP = 3,  // 0b011
	B_FLAT = -1, // 0b101
	B_WALL = -2, // 0b110
	B_CAP = -3,  // 0b111
};

inline bool is_flat(Piece p) {
	return p == Piece::W_FLAT || p == Piece::B_FLAT;
}

inline bool is_wall(Piece p) {
	return p == Piece::W_WALL || p == Piece::B_WALL;
}

inline bool is_capstone(Piece p) {
	return p == Piece::W_CAP || p == Piece::B_CAP;
}
