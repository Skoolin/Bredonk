#include <inttypes.h>

#include "move.h"

#pragma once

// MoveIterator is an abstract base class for iterating over moves in a Tak game.
class MoveIterator {
public:
	virtual move_t get_next() = 0;
};
