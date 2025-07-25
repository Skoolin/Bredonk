#include <inttypes.h>
#include "piece.h"

#pragma once

constexpr auto MoveList_MAX_MOVES = 2001; // can be big because only allocated at startup.. coalesced indexing for better caching.;

// piece type:
// 0 = none (empty square)
// 1 = flat
// 2 = wall
// 3 = capstone

// direction:
// 0 = up (+)
// 1 = right (>)
// 2 = down (-)
// 3 = left (<)

struct move_t { // 16 bit encoding of move, can encode moves up to board size 8
	uint8_t square_and_type; // 2 bit type/direction, 6 bit square index
	uint8_t spread_perm;     // encoding of the spread. if 0, this is placement type else spread.

	// returns true if this is a spread and false if this is a placement.
	inline bool is_spread() {
		return spread_perm != 0;
	}

	// returns the index of the starting/placement square
	inline int square_idx() {
		return square_and_type & 0b00111111;
	}

	// returns if the piece placed is a capstone
	inline bool is_cap_placement() {
		return (square_and_type >> 6) == 3;
	}

	// returns the integer representation of the piece type
	inline Piece piece_type(int current_player) {
		int8_t piece_as_int = ((int8_t) (square_and_type >> 6)) * current_player;
		return static_cast<Piece>(piece_as_int);
	}

	// returns the integer representation of the direction
	inline int spread_direction() {
		return square_and_type >> 6;
	}
};