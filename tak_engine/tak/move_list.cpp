#include "move_list.h"

MoveList::MoveList() :
	move_count(0),
	spread_count(0),
	current_index(0),
	placement_done(false),
	moves(),
	spreads()
{
}

MoveList::~MoveList()
{
}
