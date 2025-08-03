#include "move_list.h"

MoveList::MoveList() :
	move_count(0),
	current_index(0),
	placement_done(false),
	moves(),
	spreads()
{
}

MoveList::~MoveList()
{
}

move_t MoveList::next()
{
	// assuming has_next() is called before this method
	if (placement_done) {
		move_t m = spreads[current_index].next();
		if (!spreads[current_index].has_next())
			current_index++;
		return m;
	}
	move_t m = moves[current_index++];
	if (current_index >= move_count) {
		placement_done = true;
		current_index = 0;
	}
	return m;
}

bool MoveList::has_next() const
{
	return placement_done ? (current_index < spread_count && spreads[current_index].has_next()) : current_index < move_count;
}

void MoveList::add_move(move_t m)
{
	moves[move_count++] = m;
}

void MoveList::add_spread(SpreadIterator i)
{
	spreads[spread_count++] = i;
}

void MoveList::clear()
{
	move_count = 0;
	spread_count = 0;
	current_index = 0;
	placement_done = false;
}

uint32_t MoveList::size()
{
	auto count = move_count;
	for (int i = 0; i < spread_count; i++) {
		count += spreads[i].count();
	}
	return count;
}

bool MoveList::is_empty() const
{
	return move_count == 0;
}

void MoveList::reset()
{
	current_index = 0;
}