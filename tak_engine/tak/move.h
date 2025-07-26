#include <inttypes.h>
#include "piece.h"

#pragma once

constexpr auto MoveList_MAX_MOVES = 2001; // can be big because only allocated at startup.. coalesced indexing for better caching.

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

// move_t is a struct that represents a move in the game of Tak.
// 16 bit encoding of move, can encode moves up to board size 8
struct move_t { 
	uint8_t square_and_type; // 2 bit type/direction, 6 bit square index
	uint8_t spread_perm;     // encoding of the spread. if 0, this is placement type else spread.

	// returns true if this is a spread and false if this is a placement.
	inline bool is_spread() const {
		return spread_perm != 0;
	}

	// returns the index of the starting/placement square
	inline int square_idx() const {
		return square_and_type & 0b00111111;
	}

	// returns if the piece placed is a capstone
	inline bool is_cap_placement() const {
		return (square_and_type >> 6) == 3;
	}

	// returns the integer representation of the piece type
	inline Piece piece_type(int current_player) const {
		int8_t piece_as_int = ((int8_t) (square_and_type >> 6)) * current_player;
		return static_cast<Piece>(piece_as_int);
	}

	// returns the integer representation of the direction
	inline int spread_direction() const {
		return square_and_type >> 6;
	}

	inline int spread_distance() const {
		return __builtin_popcount(spread_perm);
	}
};
