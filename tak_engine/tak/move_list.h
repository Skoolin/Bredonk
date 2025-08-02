#pragma once

#include "move_iterator.h"

constexpr int MoveList_MAX_MOVES = 2001; // Maximum number of moves in a MoveList, coalesced for cache efficiency

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
	uint32_t size() const;
private:
	move_t moves[MoveList_MAX_MOVES];
	uint32_t move_count;
	uint32_t current_index;
};
