#include <inttypes.h>

#include "move.h"

#pragma once

class MoveIterator {
public:
	move_t get_next();
};