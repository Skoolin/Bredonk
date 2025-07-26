#include "move_list.h"
#include <cassert>

MoveList::MoveList() :
	move_count(0),
	current_index(0)
{
}

MoveList::~MoveList()
{
}

move_t MoveList::get_next()
{
	assert(current_index < MoveList_MAX_MOVES, "MoveList::get_next() called when no more moves are available.");
	return moves[current_index++];
}

void MoveList::add_move(move_t m)
{
	assert(move_count < MoveList_MAX_MOVES, "MoveList::add_move() called when move list is full.");
	moves[move_count++] = m;
}

void MoveList::clear()
{
	move_count = 0;
	current_index = 0;
}

int32_t MoveList::size() const
{
	return move_count;
}

bool MoveList::is_empty() const
{
	return move_count == 0;
}
