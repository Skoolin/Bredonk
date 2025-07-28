#include <inttypes.h>

#include "move.h"

#pragma once

// MoveIterator is an abstract base class for iterating over moves in a Tak game.
class MoveIterator {
public:
	virtual move_t next() = 0;
	virtual void clear() = 0;
	virtual bool is_empty() const = 0;
	virtual bool has_next() const = 0;
};
