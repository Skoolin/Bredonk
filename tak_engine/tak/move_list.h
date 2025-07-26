#pragma once

#include "move_iterator.h"

// MoveList is a concrete implementation of MoveIterator that stores a list of moves.
class MoveList : public MoveIterator
{
public:
	MoveList();
	~MoveList();

	// base class methods
	move_t get_next() override;
	void clear() override;
	bool is_empty() const override;

	void add_move(move_t m);
	int32_t size() const;
private:
	move_t moves[MoveList_MAX_MOVES];
	int32_t move_count;
	int32_t current_index;
};
