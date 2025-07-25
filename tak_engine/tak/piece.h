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