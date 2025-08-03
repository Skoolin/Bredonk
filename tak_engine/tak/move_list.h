#pragma once

#include "move_iterator.h"
#include "magic.h"

constexpr int MoveList_MAX_MOVES = 192; // Maximum number of moves in a MoveList
constexpr int MoveList_MAX_SPREAD = 64;

// MoveList is a concrete implementation of MoveIterator that stores a list of moves.
class MoveList// : public MoveIterator
{
public:
	MoveList();
	~MoveList();

	// base class methods
	move_t next();
	void clear();
	void reset();
	bool is_empty() const;
	bool has_next() const;

	void add_move(move_t m);
	void add_spread(SpreadIterator i);
	uint32_t size();
private:
	SpreadIterator spreads[MoveList_MAX_SPREAD];
	move_t moves[MoveList_MAX_MOVES];
	uint8_t move_count;
	uint8_t current_index;
	uint8_t spread_count;
	bool placement_done;
};
