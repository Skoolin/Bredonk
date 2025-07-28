#include "move_list.h"

MoveList::MoveList() :
	move_count(0),
	current_index(0),
	moves()
{
}

MoveList::~MoveList()
{
}

move_t MoveList::next()
{
	// assuming has_next() is called before this method
	return moves[current_index++];
}

bool MoveList::has_next() const
{
	return current_index < move_count;
}

void MoveList::add_move(move_t m)
{
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
